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
    bool result = gst_bin_add( GST_BIN( mElement ), element->mElement );

    if ( result )
    {
        mElements.push_back( element );
    }

    return result;
}

bool CGstPipeline::removeElement(CGstElement *element)
{
    bool result = gst_bin_remove( GST_BIN( mElement ), element->mElement );

    if ( result )
    {
        mElements.remove( element );
    }
    return result;
}

bool CGstPipeline::linkAllElements()
{
    bool isPairInitialized  = false;
    bool result = true;

    GstElemPtrIter it = mElements.begin();

    CGstElement *element1 = 0;
    CGstElement *element2 = 0;

    while ( it != mElements.end() )
    {
        element2 = *it;

        if ( isPairInitialized )
        {
            result = gst_element_link( GST_ELEMENT( element1->mElement ),
                              GST_ELEMENT( element2->mElement )
            );
            if ( !result )
            {
                break;
            }
        }

        element1 = element2;

        if ( !isPairInitialized )
        {
            isPairInitialized = true;
        }

        it++;
    }

    return result;
}

bool CGstPipeline::linkPair(CGstElement &first, CGstElement &second )
{
    bool result = gst_element_link( GST_ELEMENT( first.mElement ),
                      GST_ELEMENT( second.mElement )
    );

    return result;
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
