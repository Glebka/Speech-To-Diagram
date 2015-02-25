/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2015 Hlieb Romanov <rgewebppc at gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <curl/curl.h>
#include <alloca.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <json-glib/json-glib.h>
#include <pthread.h>

#include "gstgooglespeech.h"
#include "gstvader.h"
#include "googmarshal.h"

//GST_DEBUG_CATEGORY_STATIC (gst_google_speech_debug);
//#define GST_CAT_DEFAULT gst_google_speech_debug

/* Filter signals and args */
enum
{
   SIGNAL_ERROR,
   SIGNAL_RESULT,
   LAST_SIGNAL
};

enum
{
   PROP_0,
   PROP_KEY,
   PROP_LANG,
   PROP_APP
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE("sink",
                                                                   GST_PAD_SINK,
                                                                   GST_PAD_ALWAYS,
                                                                   GST_STATIC_CAPS("audio/x-raw, "
                                                                                   "format = (string) S16LE,"
                                                                                   "rate = (int) 8000, "
                                                                                   "channels = (int) 1, "
                                                                                   "layout=(string) interleaved")
                                                                   );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE("src",
                                                                  GST_PAD_SRC,
                                                                  GST_PAD_ALWAYS,
                                                                  GST_STATIC_CAPS("text/plain")
                                                                  );

static guint gst_google_speech_signals[LAST_SIGNAL];

#define gst_google_speech_parent_class parent_class
G_DEFINE_TYPE (GstGoogleSpeech, gst_google_speech, GST_TYPE_ELEMENT);

static void gst_google_speech_set_property (GObject * object, guint prop_id,
                                            const GValue * value, GParamSpec * pspec);
static void gst_google_speech_get_property (GObject * object, guint prop_id,
                                            GValue * value, GParamSpec * pspec);

static gboolean gst_google_speech_sink_event (GstPad * pad, GstObject * parent, GstEvent * event);
static GstFlowReturn gst_google_speech_chain (GstPad * pad, GstObject * parent, GstBuffer * buf);

static void
gst_google_speech_finalize(GObject * gobject)
{
   GstGoogleSpeech *filter = GST_GOOGLESPEECH (gobject);

   g_free( filter->key );
   g_free( filter->app );
   g_free( filter->lang );
   GST_CALL_PARENT(G_OBJECT_CLASS, finalize,(gobject));
}

/* GObject vmethod implementations */

/* initialize the googlespeech's class */
static void
gst_google_speech_class_init (GstGoogleSpeechClass * klass)
{ 
   GObjectClass *gobject_class;
   GstElementClass *gstelement_class;

   gobject_class = (GObjectClass *) klass;
   gstelement_class = (GstElementClass *) klass;

   gobject_class->set_property = gst_google_speech_set_property;
   gobject_class->get_property = gst_google_speech_get_property;
   gobject_class->finalize = GST_DEBUG_FUNCPTR(gst_google_speech_finalize);

   g_object_class_install_property
         (gobject_class, PROP_KEY,
          g_param_spec_string("key", "API key",
                              "Google Speech API key",
                              NULL,
                              G_PARAM_READWRITE));
   g_object_class_install_property
         (gobject_class, PROP_LANG,
          g_param_spec_string("lang", "Recognition language",
                              "Recognition language",
                              "en-us",
                              G_PARAM_READWRITE));
   g_object_class_install_property
         (gobject_class, PROP_APP,
          g_param_spec_string("application", "Application name",
                              "Optional application name",
                              "chrome",
                              G_PARAM_READWRITE));

   gst_google_speech_signals[SIGNAL_ERROR] =
         g_signal_new("error",
                      G_TYPE_FROM_CLASS(klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET(GstGoogleSpeechClass, error),
                      NULL, NULL,
                      goog_marshal_VOID__INT_STRING,
                      G_TYPE_NONE,
                      2, G_TYPE_INT, G_TYPE_STRING
                      );

   gst_google_speech_signals[SIGNAL_RESULT] =
         g_signal_new("result",
                      G_TYPE_FROM_CLASS(klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET(GstGoogleSpeechClass, result),
                      NULL, NULL,
                      goog_marshal_VOID__STRING,
                      G_TYPE_NONE,
                      1, G_TYPE_STRING
                      );

   gst_element_class_set_details_simple(gstelement_class,
                                        "GoogleSpeech",
                                        "Filter/Audio",
                                        "Convert speech to text using Google Speech API v2",
                                        "Hlieb Romanov <rgewebppc@gmail.com>");

   gst_element_class_add_pad_template (gstelement_class,
                                       gst_static_pad_template_get (&src_factory));
   gst_element_class_add_pad_template (gstelement_class,
                                       gst_static_pad_template_get (&sink_factory));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_google_speech_init (GstGoogleSpeech * filter)
{
   filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
   gst_pad_set_event_function (filter->sinkpad,
                               GST_DEBUG_FUNCPTR(gst_google_speech_sink_event));
   gst_pad_set_chain_function (filter->sinkpad,
                               GST_DEBUG_FUNCPTR(gst_google_speech_chain));
   gst_pad_use_fixed_caps(filter->sinkpad);
   gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

   filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
   gst_pad_use_fixed_caps(filter->srcpad);
   gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);
   filter->listening = FALSE;
   filter->buffer = NULL;
}

static void
gst_google_speech_set_property (GObject * object, guint prop_id,
                                const GValue * value, GParamSpec * pspec)
{
   GstGoogleSpeech *filter = GST_GOOGLESPEECH (object);

   switch (prop_id) {
   case PROP_KEY:
      filter->key = g_value_dup_string (value);
      break;
   case PROP_APP:
      filter->app = g_value_dup_string (value);
      break;
   case PROP_LANG:
      filter->lang = g_value_dup_string (value);
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
   }
}

static void
gst_google_speech_get_property (GObject * object, guint prop_id,
                                GValue * value, GParamSpec * pspec)
{
   GstGoogleSpeech *filter = GST_GOOGLESPEECH (object);
   switch (prop_id) {
   case PROP_KEY:
      g_value_set_string (value, filter->key);
      break;
   case PROP_APP:
      g_value_set_string (value, filter->app);
      break;
   case PROP_LANG:
      g_value_set_string (value, filter->lang);
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
   }
}

/* GstElement vmethod implementations */

#define RESPONSE_SIZE 0x100

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
   GstMapInfo * buffer = (GstMapInfo *)userp;
   size_t total = size * nmemb;
   size_t toCopy = 0;

   if (size*nmemb < 1)
      return 0;

   if ( buffer->size )
   {
      toCopy = ( total < buffer->size ) ? total : buffer->size;
      memcpy( ptr, buffer->data, toCopy );
      buffer->data += toCopy;
      buffer->size -= toCopy;
      return toCopy;
   }

   return 0;                          /* no more data left to deliver */
}

static size_t http_callback(char *ptr, size_t count, size_t blocksize, void *userdata);
static void free_response(struct server_response *resp);

static struct server_response * send_audio_data( GstGoogleSpeech *filter, GstMapInfo buffer )
{
   CURL *conn_hndl;
   struct curl_slist *headers;
   struct server_response *resp;
   char url[0x100];
   char header[0x100];

   /*
         * Initialize the variables
         * Put the language code to the URL query string
         * If no language given, default to U. S. English
         */

   snprintf(
            url,
            sizeof(url),
            "https://www.google.com/speech-api/v2/recognize?output=json&key=%s&client=%s&lang=%s",
            filter->key, filter->app, filter->lang);


   resp = malloc(sizeof(*resp));
   if (!resp)
      return NULL;

   resp->data = malloc(RESPONSE_SIZE);
   if (!resp->data)
   {
      free_response(resp);
      return NULL;
   }

   resp->length = 0;

   conn_hndl = curl_easy_init();
   if (!conn_hndl) {
      free_response(resp);
      return NULL;
   }

   headers = NULL;

   sprintf( header, "Content-Length: %lu", buffer.size );
   headers = curl_slist_append(headers, "Content-Type: audio/l16; rate=8000" );
   headers = curl_slist_append(headers, header );

   /*
         * Setup the cURL handle
         */
   curl_easy_setopt(conn_hndl, CURLOPT_URL, url);
   curl_easy_setopt(conn_hndl, CURLOPT_HTTPHEADER, headers);
   curl_easy_setopt(conn_hndl, CURLOPT_POST, 1L);
   curl_easy_setopt(conn_hndl, CURLOPT_READFUNCTION, read_callback);
   curl_easy_setopt(conn_hndl, CURLOPT_READDATA, &buffer);
   curl_easy_setopt(conn_hndl, CURLOPT_WRITEFUNCTION, http_callback);
   curl_easy_setopt(conn_hndl, CURLOPT_WRITEDATA, (void *)resp);
   //curl_easy_setopt(conn_hndl, CURLOPT_VERBOSE, 1L);

   /*
         * SSL certificates are not available on iOS, so we have to trust Google
         * (0 means false)
         */
   //curl_easy_setopt(conn_hndl, CURLOPT_SSL_VERIFYPEER, 0);

   /*
         * Initiate the HTTP(S) transfer
         */
   curl_easy_perform(conn_hndl);
   curl_easy_getinfo (conn_hndl, CURLINFO_RESPONSE_CODE, &resp->status);

   /*
         * Clean up
         */
   curl_slist_free_all(headers);
   curl_easy_cleanup(conn_hndl);

   /*
         * NULL-terminate the JSON response string
         */
   resp->data[resp->length] = '\0';

   if ( resp->status == 200 && strlen( resp->data ) > 0 )
   {
      perror(resp->data);
      JsonParser *parser = json_parser_new ();
      json_parser_load_from_data (parser, resp->data, -1, NULL);
      JsonNode *result;
      JsonPath *path = json_path_new ();
      json_path_compile (path, "$..transcript", NULL);
      result = json_path_match (path, json_parser_get_root (parser));
      JsonArray * array = json_node_get_array (result);
      JsonNode * transcriptNode = json_array_get_element( array, 0 );
      free( resp->data );
      resp->data = json_node_dup_string( transcriptNode );
      resp->length = strlen( resp->data );
      g_object_unref (parser);
      g_object_unref (path);
   }

   return resp;
}

static void free_response(struct server_response *resp)
{
   if (resp) {
      free(resp->data);
      free(resp);
   }
}

static size_t http_callback(char *ptr, size_t count, size_t blocksize, void *userdata)
{
   struct server_response *response = userdata;
   size_t size = count * blocksize;

   if ( strncmp( ptr, "{\"result\":[]}", 13 ) == 0 )
      return size;

   if (response->length + size < RESPONSE_SIZE) {
      /* do not write past buffer */
      memcpy(response->data + response->length, ptr, size);
      response->length += size;
   }

   return size;
}

static struct sprec_wav_header *sprec_wav_header_from_params(uint32_t sample_rate, uint16_t bit_depth, uint16_t channels)
{
   struct sprec_wav_header *hdr;
   hdr = malloc(sizeof(*hdr));
   if (!hdr)
      return NULL;

   memcpy(&hdr->RIFF_marker, "RIFF", 4);
   memcpy(&hdr->filetype_header, "WAVE", 4);
   memcpy(&hdr->format_marker, "fmt ", 4);
   hdr->data_header_length = 16;
   hdr->format_type = 1;
   hdr->number_of_channels = channels;
   hdr->sample_rate = sample_rate;
   hdr->bytes_per_second = sample_rate * channels * bit_depth / 8;
   hdr->bytes_per_frame = channels * bit_depth / 8;
   hdr->bits_per_sample = bit_depth;
   memcpy(&hdr->data_marker, "data", 4);

   return hdr;
}

//OVER SOUL ft. GRAMMA FUNK - Universal Unfolding

int strupp(char *s) {
    int i;
    for (i = 0; i < strlen(s); i++)
        s[i] = toupper(s[i]);
    return i;
}

static GstEvent *
gst_google_speech_event_new( GstGoogleSpeech *c,
                             GstEventType type,
                             GstClockTime timestamp,
                             const gchar* transcript )
{
    GstEvent *e;
    GstStructure* feedback
          = gst_structure_new ( "feedback",
                                "transcript", G_TYPE_STRING, transcript,
                                NULL
                              );
    gchar* transcriptDup = g_strdup( transcript );
    strupp( transcriptDup );
    gchar* ptr = strtok( transcriptDup, " " );
    int words = 0;
    char index[10];
    while ( ptr != NULL )
    {
       sprintf( index, "w%d", words );
       gst_structure_set( feedback, index, G_TYPE_STRING, ptr, NULL );
       ++words;
       ptr = strtok( NULL, " " );
    }
    gst_structure_set( feedback, "count", G_TYPE_UINT, words, NULL );
    e = gst_event_new_custom(type, feedback);
    GST_EVENT_TIMESTAMP(e) = timestamp;

    return e;
}

struct WorkerContext
{
   GstGoogleSpeech *filter;
   GstBuffer* buffer;
};

static void* asyncSendWorker( void* user_data )
{
   struct WorkerContext* context = ( struct WorkerContext* ) user_data;

   GstBuffer* wave = context->buffer;
   GstGoogleSpeech* filter = context->filter;

   GstMapInfo mapInfo;
   gst_buffer_map( wave, &mapInfo, GST_MAP_READ | GST_MAP_WRITE );

   struct server_response * resp = send_audio_data( filter, mapInfo );
   g_signal_emit(filter, gst_google_speech_signals[SIGNAL_RESULT],
                 0, g_strdup( resp->data ));
   free_response( resp );
   gst_buffer_unref( wave );
   free( context );
}

/* this function handles sink events */
static gboolean
gst_google_speech_sink_event (GstPad * pad, GstObject * parent, GstEvent * event)
{
   gboolean ret;
   GstGoogleSpeech *filter;

   filter = GST_GOOGLESPEECH (parent);

   switch (GST_EVENT_TYPE (event)) {
   case GST_EVENT_SEGMENT:
      return gst_pad_event_default(pad, parent, event);
   case GST_EVENT_VADER_START:
      filter->listening = TRUE;
      filter->buffer = gst_buffer_new();
      /* Forward this event. */
      return gst_pad_event_default(pad, parent, event);
   case GST_EVENT_EOS:
   case GST_EVENT_VADER_STOP:
   {
      filter->listening = FALSE;
      GstMapInfo mi;
      gst_buffer_map( filter->buffer, &mi, GST_MAP_READ | GST_MAP_WRITE );

      struct sprec_wav_header *hdr = sprec_wav_header_from_params( 8000, 16, 1 );
      hdr->file_size = mi.size + 44 - 8;
      hdr->data_size = hdr->file_size + 8 - 44;

      gst_buffer_unmap( filter->buffer, &mi );

      GstBuffer* header = gst_buffer_new_allocate( NULL, 44, NULL );
      GstMapInfo headerMap;
      gst_buffer_map( header, &headerMap, GST_MAP_READ | GST_MAP_WRITE );
      void *data = headerMap.data;
      memcpy( data, &hdr->RIFF_marker, 4 );
      data+=4;
      memcpy( data, &hdr->file_size, 4 );
      data+=4;
      memcpy( data, &hdr->filetype_header, 4 );
      data+=4;
      memcpy( data, &hdr->format_marker, 4 );
      data+=4;
      memcpy( data, &hdr->data_header_length, 4 );
      data+=4;
      memcpy( data, &hdr->format_type, 2 );
      data+=2;
      memcpy( data, &hdr->number_of_channels, 2 );
      data+=2;
      memcpy( data, &hdr->sample_rate, 4 );
      data+=4;
      memcpy( data, &hdr->bytes_per_second, 4 );
      data+=4;
      memcpy( data, &hdr->bytes_per_frame, 2 );
      data+=2;
      memcpy( data, &hdr->bits_per_sample, 2 );
      data+=2;
      memcpy( data, &hdr->data_marker, 4 );
      data+=4;
      memcpy( data, &hdr->data_size, 4 );
      gst_buffer_unmap( header, &headerMap );

      free( hdr );

      GstBuffer* wave = gst_buffer_append( header, filter->buffer );

      struct WorkerContext* context = malloc( sizeof( struct WorkerContext ) );
      context->buffer = wave;
      context->filter = filter;

      pthread_t thread;
      pthread_create(&thread, NULL, asyncSendWorker, context);
      pthread_detach(thread);

      filter->buffer = 0;
   }
      return gst_pad_event_default(pad, parent, event);
   default:
      ret = gst_pad_event_default (pad, parent, event);
      break;
   }
   return ret;
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_google_speech_chain (GstPad * pad, GstObject * parent, GstBuffer * buf)
{
   GstGoogleSpeech *filter;

   filter = GST_GOOGLESPEECH (parent);

   if ( filter->listening == TRUE )
   {
      filter->buffer = gst_buffer_append( filter->buffer, buf );
   }

   /* just push out the incoming buffer without touching it */
   return GST_FLOW_OK;
}
