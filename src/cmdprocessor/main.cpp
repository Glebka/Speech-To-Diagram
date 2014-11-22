#include <gst/gst.h>
#include <glib.h>
#include <config.h>

#include "CMainLoop.hpp"
#include "CGstAudioConverter.hpp"
#include "CGstAudioResampler.hpp"
#include "CGstPipeline.hpp"
#include "CGstSource.hpp"
#include "CGstSpeechRecognizer.hpp"
#include "CGstVoiceActivityDetector.hpp"
#include "CGstFakeSink.hpp"



/*static gboolean
bus_call (GstBus     *bus,
          GstMessage *msg,
          gpointer    data)
{
  GMainLoop *loop = (GMainLoop *) data;

  switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;

    case GST_MESSAGE_ERROR: {
      gchar  *debug;
      GError *error;

      gst_message_parse_error (msg, &error, &debug);


      g_printerr ("Error: %s\n%s", error->message, debug);
      g_free (debug);
      g_error_free (error);

      g_main_loop_quit (loop);
      break;
    }
    default:
      break;
  }

  return TRUE;
}


/*static void
on_pad_added (GstElement *element,
              GstPad     *pad,
              gpointer    data)
{
  GstPad *sinkpad;
  GstElement *decoder = (GstElement *) data;

  g_print ("Dynamic pad created, linking demuxer/decoder\n");

  sinkpad = gst_element_get_static_pad (decoder, "sink");

  gst_pad_link (pad, sinkpad);

  gst_object_unref (sinkpad);
}*/

/*void on_partial_result (GstElement* object,
                                          gchararray arg0,
                                          gchararray arg1,
                                          gpointer user_data)
{
    g_print ("Partial result: %s\n", arg0);
}

void on_result (GstElement* object,
                                  gchararray arg0,
                                  gchararray arg1,
                                  gpointer user_data)
{
    g_print ("Result: %s\n", arg0);
}

void on_vader_start (GstElement* object,
                                       guint64 arg0,
                                       gpointer user_data)
{
    g_print ("Vader started\n");
}

void on_vader_stop (GstElement* object,
                                      guint64 arg0,
                                      gpointer user_data)
{
    g_print ("Vader stopped\n");
}*/

//alsasrc ! audioconvert ! audioresample ! vader name=vad auto_threshold=true ! pocketsphinx name=asr ! fakesink'

int main ( int argc, char *argv[] )
{
    CMainLoop loop( argc, argv );

    CGstPipeline pipeline( loop );

    CGstSource source;
    CGstAudioConverter converter;
    CGstAudioResampler resampler;

    CGstVoiceActivityDetector vader;
    vader.setAutoThreshold( true );

    CGstSpeechRecognizer recognizer;

    recognizer.setAcousticModelDirectoryPath( PROJECT_ACOUSTIC_DATA_DIR );
    recognizer.setDictionaryFilePath( PROJECT_LANG_DATA_DIR "/" PROJECT_LM_DIR_NAME "/" PROJECT_LM_DIR_NAME ".dic" );
    recognizer.setLanguageModelFilePath( PROJECT_LANG_DATA_DIR "/" PROJECT_LM_DIR_NAME "/" PROJECT_LM_DIR_NAME PROJECT_LM_FILE_EXTENSION );

    CGstSpeechRecognizer recognizer2;

    recognizer2.setAcousticModelDirectoryPath( PROJECT_ACOUSTIC_DATA_DIR );
    recognizer2.setDictionaryFilePath( PROJECT_LANG_DATA_DIR "/" PROJECT_LM_DIR_NAME "/" PROJECT_LM_DIR_NAME ".dic" );
    recognizer2.setLanguageModelFilePath( PROJECT_LANG_DATA_DIR "/" PROJECT_LM_DIR_NAME "/" PROJECT_LM_DIR_NAME PROJECT_LM_FILE_EXTENSION );

    CGstFakeSink sink;

    pipeline.addElement( &source );
    pipeline.addElement( &converter );
    pipeline.addElement( &resampler );
    pipeline.addElement( &vader );
    pipeline.addElement( &recognizer );
    pipeline.addElement( &sink );

    pipeline.linkAllElements();

    pipeline.start();

    loop.run();

    pipeline.stop();
  /*GMainLoop *loop;

  GstElement *pipeline, *source, *converter, *resampler, *vader, *recognizer, *sink;
  GstBus *bus;
  guint bus_watch_id;


  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);



  pipeline = gst_pipeline_new ("sphinx");
  source   = gst_element_factory_make (AUDIO_SOURCE_PLUGIN, AUDIO_SOURCE_PLUGIN);
  converter  = gst_element_factory_make ("audioconvert",     "converter");
  resampler  = gst_element_factory_make ("audioresample",     "resampler");
  vader     = gst_element_factory_make ("vader",  "vad");
  recognizer     = gst_element_factory_make ("pocketsphinx", "recognizer");
  sink     = gst_element_factory_make ("fakesink", "sink");

  if (!pipeline || !source || !converter || !resampler || !vader || !recognizer || !sink) {
  //if (!pipeline || !source || !converter || !resampler || !vader || !sink) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }


  g_object_set (G_OBJECT (vader), "auto-threshold",TRUE, NULL);
  g_object_set (G_OBJECT (recognizer), "hmm", PROJECT_ACOUSTIC_DATA_DIR, NULL);
  g_object_set (G_OBJECT (recognizer), "dict", PROJECT_LANG_DATA_DIR "/" PROJECT_LM_DIR_NAME "/" PROJECT_LM_DIR_NAME ".dic", NULL);
  g_object_set (G_OBJECT (recognizer), "lm", PROJECT_LANG_DATA_DIR "/" PROJECT_LM_DIR_NAME "/" PROJECT_LM_DIR_NAME PROJECT_LM_FILE_EXTENSION, NULL);


  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);


  gst_bin_add_many (GST_BIN (pipeline),
                    source, converter, resampler, vader, recognizer, sink, NULL);
  //gst_bin_add_many (GST_BIN (pipeline),
  //                    source, converter, resampler, vader, sink, NULL);

  gst_element_link_many (source, converter,resampler, vader, recognizer, sink, NULL);
  //gst_element_link_many (source, converter,resampler, vader, sink, NULL);
  g_signal_connect (vader, "vader-start", G_CALLBACK (on_vader_start), NULL);
  g_signal_connect (vader, "vader-stop", G_CALLBACK (on_vader_stop), NULL);
  g_signal_connect (recognizer, "partial_result", G_CALLBACK (on_partial_result), sink);
  g_signal_connect (recognizer, "result", G_CALLBACK (on_result), sink);


  g_print ("Now playing\n");
  gst_element_set_state (pipeline, GST_STATE_PLAYING);



  g_print ("Running...\n");
  g_main_loop_run (loop);



  g_print ("Returned, stopping\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);

  g_print ("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT (pipeline));
  g_source_remove (bus_watch_id);
  g_main_loop_unref (loop);*/

  return 0;
}
