/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2015  <<user@hostname.org>>
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

/**
 * SECTION:element-googlespeech
 *
 * FIXME:Describe googlespeech here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! googlespeech ! fakesink silent=TRUE
 * ]|
 * </refsect2>
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

    /*g_hash_table_foreach(ps->arghash, string_disposal, NULL);
    g_hash_table_destroy(ps->arghash);
    g_free(ps->last_result);
    ps_free(ps->ps);
    cmd_ln_free_r(ps->config);*/
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

#define BUF_SIZE 0x1000
#define RESPONSE_SIZE 0x100

struct MemoryStruct {
  char *memory;
  size_t size;
};
 
 
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */ 
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    char buf[100];
     snprintf(
                buf,
                100,
                "Size:%d, nmemb: %d\n",
                size, nmemb);
    perror(buf);
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

struct WriteThis {
    const char *readptr;
    long sizeleft;
};

// callback function from http://curl.haxx.se/libcurl/c/post-callback.html
static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
    perror("Pooh\n");
    struct WriteThis *pooh = (struct WriteThis *)userp;

    if (size*nmemb < 1)
        return 0;

    if (pooh->sizeleft) {
        *(char *)ptr = pooh->readptr[0]; /* copy one single byte */
        pooh->readptr++;                 /* advance pointer */
        pooh->sizeleft--;                /* less data left */
        return 1;                        /* we return 1 byte at a time! */
    }

    return 0;                          /* no more data left to deliver */
}

static size_t http_callback(char *ptr, size_t count, size_t blocksize, void *userdata);
static void free_response(struct server_response *resp);

static char * send_audio_data( GstGoogleSpeech *filter, void *data, int length)
{
        CURL *conn_hndl;
        struct curl_httppost *form, *lastptr;
        struct curl_slist *headers;
        struct server_response *resp;
        char url[0x100];
        char header[0x100];
        
        if (!data)
                return NULL;

        /*
         * Initialize the variables
         * Put the language code to the URL query string
         * If no language given, default to U. S. English
         */

    snprintf(
                url,
                sizeof(url),
                "https://www.google.com/speech-api/v2/recognize?output=json&key=%s&client=%s&lang=%s&pfilter=2",
                filter->key, filter->app, filter->lang);

    
         char template[100];
         strcpy( template, "/tmp/googXXXXXX" );
         char* tmp_name = mktemp( template );
         strcat( tmp_name, ".txt" );
         FILE* f = fopen( tmp_name, "wb" );
         //fwrite( mapInfo.data, 1, mapInfo.size, f );
            
        /*resp = malloc(sizeof(*resp));
        if (!resp)
                return NULL;
        
        resp->data = malloc(RESPONSE_SIZE);
        if (!resp->data) {
                free_response(resp);
                return NULL;
        }
        
        resp->length = 0;
        */
        
        /*struct MemoryStruct chunk;
 
        chunk.memory = malloc(BUF_SIZE);  /* will be grown as needed by the realloc above */ 
        //chunk.size = 0;    /* no data at this point */ 
        
        conn_hndl = curl_easy_init();
        if (!conn_hndl) {
                //free_response(resp);
                //free(chunk.memory);
                return NULL;
        }
        
        form = NULL;
        lastptr = NULL;
        headers = NULL;
        struct WriteThis pooh;
        
        pooh.readptr = data;
        pooh.sizeleft = length;
        
        sprintf( header, "Content-Length: %u", length );
        headers = curl_slist_append(headers, "Content-Type: audio/l16; rate=8000" );
        headers = curl_slist_append(headers, header );
        
        //headers = curl_slist_append(headers, "User-Agent: curl/7.40.0");
        //headers = curl_slist_append(headers, "Keep-Alive: timeout=5000");
        //headers = curl_slist_append(headers, "Connection: Keep-Alive");
        

        /*curl_formadd(
                &form,
                &lastptr,
                CURLFORM_COPYNAME,
                "myfile",
                CURLFORM_CONTENTSLENGTH,
                length,
                CURLFORM_PTRCONTENTS,
                data,
                CURLFORM_END
        );
*/
        /*
         * Setup the cURL handle
         */
        curl_easy_setopt(conn_hndl, CURLOPT_URL, url);
        curl_easy_setopt(conn_hndl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(conn_hndl, CURLOPT_POST, 1L);
        curl_easy_setopt(conn_hndl, CURLOPT_READFUNCTION, read_callback);
        curl_easy_setopt(conn_hndl, CURLOPT_READDATA, &pooh);
        curl_easy_setopt(conn_hndl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(conn_hndl, CURLOPT_WRITEDATA, (void *)f);
        curl_easy_setopt(conn_hndl, CURLOPT_VERBOSE, 1L);
        //curl_easy_setopt(conn_hndl, CURLOPT_EXPECT_100_TIMEOUT_MS, 5000);
        //curl_easy_setopt(conn_hndl, CURLOPT_HTTP_TRANSFER_DECODING, 0);        

        /*
         * SSL certificates are not available on iOS, so we have to trust Google
         * (0 means false)
         */
        //curl_easy_setopt(conn_hndl, CURLOPT_SSL_VERIFYPEER, 0);

        /*
         * Initiate the HTTP(S) transfer
         */
        curl_easy_perform(conn_hndl);

        /*
         * Clean up
         */
        curl_formfree(form);
        curl_slist_free_all(headers);
        curl_easy_cleanup(conn_hndl);
        fclose(f);
        /*
         * NULL-terminate the JSON response string
         */
        //resp->data[resp->length] = '\0';

        return g_strdup(tmp_name);
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
            
            GstMapInfo mapInfo;
            gst_buffer_map( wave, &mapInfo, GST_MAP_READ | GST_MAP_WRITE );
            
            
            
            char* resp = send_audio_data( filter, mapInfo.data, mapInfo.size );
            g_signal_emit(filter, gst_google_speech_signals[SIGNAL_RESULT],
                              0, g_strdup( resp ));
            free( resp );
            /*gst_buffer_unmap( filter->buffer, &mapInfo );
            gst_buffer_unref( filter->buffer );*/
            //gst_buffer_unref( header );
            gst_buffer_unref( wave );
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