#include <gst/gst.h>
#include <glib.h>
#include <config.h>

// обработчик событий цепочки вцелом
static gboolean
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

// обработчик события partial_result
// вызывается при появлении нового промежуточного результата распознавания
void on_partial_result (GstElement* object,
                                          gchararray arg0,
                                          gchararray arg1,
                                          gpointer user_data)
{
	// вывод частично распознанной фразы на консоль
    g_print ("Partial result: %s\n", arg0);
}

// обработчик события result
// вызывается, когда произнесенная фраза полностью распознана
void on_result (GstElement* object,
                                  gchararray arg0,
                                  gchararray arg1,
                                  gpointer user_data)
{
	// вывод на консоль распознанной фразы
    g_print ("Result: %s\n", arg0);
}

// обработчик события срабатывания элемента Voice Activity Detector
// после срабатывания элемента Voice Activity Detector аудиопоток начинает поступать на элемент pocketsphinx
void on_vader_start (GstElement* object,
                                       guint64 arg0,
                                       gpointer user_data)
{
    g_print ("Vader started\n");
}

// обработчик события остановки элемента Voice Activity Detector
// Voice Activity Detector перестает передавать аудиопоток на распознавание после того, 
// как в микрофон перестает поступать звук, громкость которого не превышает заданный порог
void on_vader_stop (GstElement* object,
                                      guint64 arg0,
                                      gpointer user_data)
{
    g_print ("Vader stopped\n");
}

// главная функция приложения
int
main (int   argc,
      char *argv[])
{

  // объявление указателя на цикл обработки событий
  GMainLoop *loop;

  // объявление указателей на элементы цепочки GStreamer
  GstElement *pipeline, *source, *converter, *resampler, *vader, *recognizer, *sink;
  GstBus *bus;
  guint bus_watch_id;

  // инициализация GStreamer
  gst_init (&argc, &argv);

  // создание цикла обработки событий
  loop = g_main_loop_new (NULL, FALSE);


  // создание новой цепочки и элементов цепочки
  pipeline = gst_pipeline_new ("sphinx");

  // создание источника аудиопотока (задается через конфигурацию сборки)
  source   = gst_element_factory_make (AUDIO_SOURCE_PLUGIN, AUDIO_SOURCE_PLUGIN);

  // создание аудиоконвертера и ресемплера
  converter  = gst_element_factory_make ("audioconvert",     "converter");
  resampler  = gst_element_factory_make ("audioresample",     "resampler");

  // созадние элемента Voice Activity Detector
  vader     = gst_element_factory_make ("vader",  "vad");

  // созадние элемента pocketsphinx для распознавания речи
  recognizer     = gst_element_factory_make ("pocketsphinx", "recognizer");

  // создание стока
  sink     = gst_element_factory_make ("fakesink", "sink");

  if (!pipeline || !source || !converter || !resampler || !vader || !recognizer || !sink) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }

  // настройка элементов цепочки

  // установка автоматического порога для Voice Activity Detector
  g_object_set (G_OBJECT (vader), "auto-threshold",TRUE, NULL);

  // установка пути, по которому расположена акустическая модель языка
  // путь указан в файле конфигурации сборки
  g_object_set (G_OBJECT (recognizer), "hmm", PROJECT_ACOUSTIC_DATA_DIR, NULL);

  // установка пути к файлу фонетического словаря
  // путь указан в файле конфигурации сборки
  g_object_set (G_OBJECT (recognizer), "dict", PROJECT_LANG_DATA_DIR "/" PROJECT_LM_DIR_NAME "/" PROJECT_LM_DIR_NAME ".dic", NULL);

  // установка пути к файлу статистической языковой модели
  // путь указан в файле конфигурации сборки
  g_object_set (G_OBJECT (recognizer), "lm", PROJECT_LANG_DATA_DIR "/" PROJECT_LM_DIR_NAME "/" PROJECT_LM_DIR_NAME PROJECT_LM_FILE_EXTENSION, NULL);

  // установка обрботчика событий цепочки вцелом
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  
  // добавление элементов в цепочку
  gst_bin_add_many (GST_BIN (pipeline),
                    source, converter, resampler, vader, recognizer, sink, NULL);
  
  // связывание элементов в цепочке
  gst_element_link_many (source, converter,resampler, vader, recognizer, sink, NULL);
  
  // установка обработчиков событий элемента Voice Activity Detector
  g_signal_connect (vader, "vader-start", G_CALLBACK (on_vader_start), NULL);
  g_signal_connect (vader, "vader-stop", G_CALLBACK (on_vader_stop), NULL);

  // установка обработчиков событий pocketsphinx
  g_signal_connect (recognizer, "partial_result", G_CALLBACK (on_partial_result), sink);
  g_signal_connect (recognizer, "result", G_CALLBACK (on_result), sink);

  // запуск цепочки
  g_print ("Now playing\n");
  gst_element_set_state (pipeline, GST_STATE_PLAYING);


  /* Iterate */
  g_print ("Running...\n");
  g_main_loop_run (loop);


  /* Out of the main loop, clean up nicely */
  g_print ("Returned, stopping\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);

  g_print ("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT (pipeline));
  g_source_remove (bus_watch_id);
  g_main_loop_unref (loop);

  return 0;
}
