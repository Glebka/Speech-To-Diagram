#include "CGstPipeline.hpp"

static gboolean busHandler( GstBus *bus, GstMessage *msg, gpointer data )
{
  CMainLoop *loop = ( CMainLoop* ) data;

  switch (GST_MESSAGE_TYPE ( msg )) {

    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      loop->quit();
      break;

    case GST_MESSAGE_ERROR: {
      gchar  *debug;
      GError *error;

      gst_message_parse_error (msg, &error, &debug);


      g_printerr ("Error: %s\n%s", error->message, debug);
      g_free (debug);
      g_error_free (error);

      loop->quit();

      break;
    }
    default:
      break;
  }

  return TRUE;
}

CGstPipeline::CGstPipeline( CMainLoop &loop )
    : CGstElement( gst_pipeline_new( NULL ) )
    , mLoop( loop )
{
    setBusHandler();
}

CGstPipeline::~CGstPipeline()
{
}

bool CGstPipeline::addElement(CGstElement *element)
{
    return gst_bin_add( GST_BIN( mElement ), element->mElement );
}

bool CGstPipeline::removeElement(CGstElement *element)
{
    return gst_bin_remove( GST_BIN( mElement ), element->mElement );
}

bool CGstPipeline::linkElements()
{
    GstIterator *it = gst_bin_iterate_elements( GST_BIN( mElement ) );
    bool done = false;
    bool isPairInitialized  = false;

    gpointer item1 = 0;
    gpointer item2 = 0;

    //g_value_unset( &item1 );
    //g_value_unset( &item2 );

    while (!done) {
      switch (gst_iterator_next ( it, &item2 )) {
        case GST_ITERATOR_OK:

          if ( isPairInitialized )
          {
              GstElement *el1 = GST_ELEMENT( item1 );
              GstElement *el2 = GST_ELEMENT( item2 );

              gchar *name1 = gst_element_get_name( el1 );
              gchar *name2 = gst_element_get_name( el2 );
              g_print( "%s->%s\n", name1, name2 );
              g_free( name1 );
              g_free( name2 );
              gst_element_link(
                          GST_ELEMENT( item1 ) ,
                          GST_ELEMENT( item2 )
                          );
          }

          item1 = item2;

          if ( !isPairInitialized )
          {
              isPairInitialized = true;
          }

          //g_value_reset (&item2);

          break;
        case GST_ITERATOR_RESYNC:
          gst_iterator_resync (it);
          break;
        case GST_ITERATOR_ERROR:
          done = true;
          break;
        case GST_ITERATOR_DONE:
          done = true;
          break;
      }
    }

    //g_value_unset (&item1);
    //g_value_unset (&item2);
    gst_iterator_free (it);
}

void CGstPipeline::start()
{
    gst_element_set_state ( mElement, GST_STATE_PLAYING );
}

void CGstPipeline::pause()
{
    gst_element_set_state ( mElement, GST_STATE_PAUSED );
}

void CGstPipeline::stop()
{
    gst_element_set_state ( mElement, GST_STATE_NULL );
}

void CGstPipeline::setBusHandler()
{
    GstBus *bus = gst_pipeline_get_bus ( GST_PIPELINE ( mElement ) );

    guint bus_watch_id = gst_bus_add_watch (bus, busHandler, &mLoop);

    gst_object_unref ( bus );
}
