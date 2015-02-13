/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 2007 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 * Author: David Huggins-Daines <dhuggins@cs.cmu.edu>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>
#include <gst/gst.h>

#include <sphinxbase/strfuncs.h>

#include "gstpocketsphinx.h"
#include "gstvader.h"
#include "gstgooglespeech.h"
#include "psmarshal.h"

GST_DEBUG_CATEGORY_STATIC (gst_pocket_sphinx_debug);
#define GST_CAT_DEFAULT gst_pocket_sphinx_debug

enum
{
    SIGNAL_PARTIAL_RESULT,
    SIGNAL_RESULT,
    LAST_SIGNAL
};

enum
{
    PROP_0,
    PROP_HMM_DIR,
    PROP_LM_FILE,
    PROP_LMCTL_FILE,
    PROP_LM_NAME,
    PROP_DICT_FILE,
    PROP_MLLR_FILE,
    PROP_FSG_FILE,
    PROP_FSG_MODEL,
    PROP_FWDFLAT,
    PROP_BESTPATH,
    PROP_MAXHMMPF,
    PROP_MAXWPF,
    PROP_BEAM,
    PROP_WBEAM,
    PROP_PBEAM,
    PROP_DSRATIO,
    PROP_LATDIR,
    PROP_LATTICE,
    PROP_NBEST,
    PROP_NBEST_SIZE,
    PROP_DECODER,
    PROP_CONFIGURED
};

/*
 * Static data.
 */

/* Default command line. (will go away soon and be constructed using properties) */
static char *default_argv[] = {
    "gst-pocketsphinx",
    "-samprate", "8000",
    "-cmn", "prior",
    "-fwdflat", "no",
    "-bestpath", "no",
    "-maxhmmpf", "2000",
    "-maxwpf", "20"
};
static const int default_argc = sizeof(default_argv)/sizeof(default_argv[0]);

static GstStaticPadTemplate sink_factory =
    GST_STATIC_PAD_TEMPLATE("sink",
                            GST_PAD_SINK,
                            GST_PAD_ALWAYS,
                            GST_STATIC_CAPS("audio/x-raw, "
                                             "format = (string) S16LE,"                                            
                                             "rate = (int) 8000, "
                                             "channels = (int) 1, "
                                             "layout=(string) interleaved")
        );

static GstStaticPadTemplate src_factory =
    GST_STATIC_PAD_TEMPLATE("src",
                            GST_PAD_SRC,
                            GST_PAD_ALWAYS,
                            GST_STATIC_CAPS("text/plain")
        );

static guint gst_pocketsphinx_signals[LAST_SIGNAL];

/*
 * Boxing of ps_lattice_t.
 */

GType
ps_lattice_get_type(void)
{
    static GType ps_lattice_type = 0;

    if (G_UNLIKELY(ps_lattice_type == 0)) {
        ps_lattice_type = g_boxed_type_register_static
            ("PSLattice",
             /* Conveniently, these should just work. */
             (GBoxedCopyFunc) ps_lattice_retain,
             (GBoxedFreeFunc) ps_lattice_free);
    }

    return ps_lattice_type;
}

/*
 * Boxing of ps_decoder_t.
 */

GType
ps_decoder_get_type(void)
{
    static GType ps_decoder_type = 0;

    if (G_UNLIKELY(ps_decoder_type == 0)) {
        ps_decoder_type = g_boxed_type_register_static
            ("PSDecoder",
             /* Conveniently, these should just work. */
             (GBoxedCopyFunc) ps_retain,
             (GBoxedFreeFunc) ps_free);
    }

    return ps_decoder_type;
}

#define gst_pocket_sphinx_parent_class parent_class
G_DEFINE_TYPE (GstPocketSphinx, gst_pocket_sphinx, GST_TYPE_ELEMENT);

static void gst_pocket_sphinx_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_pocket_sphinx_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_pocket_sphinx_sink_event (GstPad * pad, GstObject * parent, GstEvent * event);
static GstFlowReturn gst_pocket_sphinx_chain (GstPad * pad, GstObject * parent, GstBuffer * buf);


static void
string_disposal(gpointer key, gpointer value, gpointer user_data)
{
    g_free(value);
}

static void
gst_pocketsphinx_finalize(GObject * gobject)
{
    GstPocketSphinx *ps = GST_POCKETSPHINX(gobject);

    g_hash_table_foreach(ps->arghash, string_disposal, NULL);
    g_hash_table_destroy(ps->arghash);
    g_free(ps->last_result);
    ps_free(ps->ps);
    cmd_ln_free_r(ps->config);
    GST_CALL_PARENT(G_OBJECT_CLASS, finalize,(gobject));
}

/* GObject vmethod implementations */

/* initialize the pocketsphinx's class */
static void
gst_pocket_sphinx_class_init (GstPocketSphinxClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_pocket_sphinx_set_property;
  gobject_class->get_property = gst_pocket_sphinx_get_property;
  gobject_class->finalize = GST_DEBUG_FUNCPTR(gst_pocketsphinx_finalize);

   /* TODO: We will bridge cmd_ln.h properties to GObject
     * properties here somehow eventually. */
    g_object_class_install_property
        (gobject_class, PROP_HMM_DIR,
         g_param_spec_string("hmm", "HMM Directory",
                             "Directory containing acoustic model parameters",
                             NULL,
                             G_PARAM_READWRITE));
    g_object_class_install_property
        (gobject_class, PROP_LM_FILE,
         g_param_spec_string("lm", "LM File",
                             "Language model file",
                             NULL,
                             G_PARAM_READWRITE));
    g_object_class_install_property
        (gobject_class, PROP_LMCTL_FILE,
         g_param_spec_string("lmctl", "LM Control File",
                             "Language model control file (for class LMs)",
                             NULL,
                             G_PARAM_READWRITE));
    g_object_class_install_property
        (gobject_class, PROP_LM_NAME,
         g_param_spec_string("lmname", "LM Name",
                             "Language model name (to select LMs from lmctl)",
                             NULL,
                             G_PARAM_READWRITE));
    g_object_class_install_property
        (gobject_class, PROP_FSG_FILE,
         g_param_spec_string("fsg", "FSG File",
                             "Finite state grammar file",
                             NULL,
                             G_PARAM_READWRITE));
    g_object_class_install_property
        (gobject_class, PROP_FSG_MODEL,
         g_param_spec_pointer("fsg_model", "FSG Model",
                              "Finite state grammar object (fsg_model_t *)",
                              G_PARAM_WRITABLE));
    g_object_class_install_property
        (gobject_class, PROP_DICT_FILE,
         g_param_spec_string("dict", "Dictionary File",
                             "Dictionary File",
                             NULL,
                             G_PARAM_READWRITE));
    g_object_class_install_property
        (gobject_class, PROP_MLLR_FILE,
         g_param_spec_string("mllr", "MLLR file",
                             "MLLR file",
                             NULL,
                             G_PARAM_READWRITE));
    g_object_class_install_property
        (gobject_class, PROP_FWDFLAT,
         g_param_spec_boolean("fwdflat", "Flat Lexicon Search",
                              "Enable Flat Lexicon Search",
                              FALSE,
                              G_PARAM_READWRITE));
    g_object_class_install_property
        (gobject_class, PROP_BESTPATH,
         g_param_spec_boolean("bestpath", "Graph Search",
                              "Enable Graph Search",
                              FALSE,
                              G_PARAM_READWRITE));

    g_object_class_install_property
        (gobject_class, PROP_LATDIR,
         g_param_spec_string("latdir", "Lattice Directory",
                             "Output Directory for Lattices",
                             NULL,
                             G_PARAM_READWRITE));
    g_object_class_install_property
        (gobject_class, PROP_LATTICE,
         g_param_spec_boxed("lattice", "Word Lattice",
                            "Word lattice object for most recent result",
                            PS_LATTICE_TYPE,
                            G_PARAM_READABLE));
    g_object_class_install_property
        (gobject_class, PROP_NBEST,
         g_param_spec_value_array("nbest", "N-best results",
                          "N-best results",
                          g_param_spec_string("nbest-hyp", "N-best hyp",
                            "N-best hyp",
                            NULL,
                            G_PARAM_READABLE),
                          G_PARAM_READABLE));  
    g_object_class_install_property
        (gobject_class, PROP_NBEST_SIZE,
         g_param_spec_int("nbest_size", "Size of N-best list",
                          "Number of hypothesis in the N-best list",
                          1, 1000, 10,
                          G_PARAM_READWRITE));
    g_object_class_install_property
        (gobject_class, PROP_MAXHMMPF,
         g_param_spec_int("maxhmmpf", "Maximum HMMs per frame",
                          "Maximum number of HMMs searched per frame",
                          1, 100000, 1000,
                          G_PARAM_READWRITE));
    g_object_class_install_property
        (gobject_class, PROP_MAXWPF,
         g_param_spec_int("maxwpf", "Maximum words per frame",
                          "Maximum number of words searched per frame",
                          1, 100000, 10,
                          G_PARAM_READWRITE));
    g_object_class_install_property
        (gobject_class, PROP_BEAM,
         g_param_spec_float("beam", "Beam width applied to every frame in Viterbi search",
                          "Beam width applied to every frame in Viterbi search",
                          -1, 1, 1e-48,
                          G_PARAM_READWRITE));
    g_object_class_install_property
        (gobject_class, PROP_PBEAM,
         g_param_spec_float("pbeam", "Beam width applied to phone transitions",
                          "Beam width applied to phone transitions",
                          -1, 1, 1e-48,
                          G_PARAM_READWRITE));
    g_object_class_install_property
        (gobject_class, PROP_WBEAM,
         g_param_spec_float("wbeam", "Beam width applied to word exits",
                          "Beam width applied to phone transitions",
                          -1, 1, 7e-29,
                          G_PARAM_READWRITE));
    g_object_class_install_property
        (gobject_class, PROP_DSRATIO,
         g_param_spec_int("dsratio", "Frame downsampling ratio",
                          "Evaluate acoustic model every N frames",
                          1, 10, 1,
                          G_PARAM_READWRITE));
    g_object_class_install_property
        (gobject_class, PROP_DECODER,
         g_param_spec_boxed("decoder", "Decoder object",
                            "The underlying decoder",
                            PS_DECODER_TYPE,
                            G_PARAM_READABLE));
    g_object_class_install_property
        (gobject_class, PROP_CONFIGURED,
         g_param_spec_boolean("configured", "Finalize configuration",
                              "Set this to finalize configuration",
                              FALSE,
                              G_PARAM_READWRITE));
    
        gst_pocketsphinx_signals[SIGNAL_PARTIAL_RESULT] = 
        g_signal_new("partial",
                     G_TYPE_FROM_CLASS(klass),
                     G_SIGNAL_RUN_LAST,
                     G_STRUCT_OFFSET(GstPocketSphinxClass, partial_result),
                     NULL, NULL,
                     ps_marshal_VOID__STRING_STRING_INT,
                     G_TYPE_NONE,
                     3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT
            );

    gst_pocketsphinx_signals[SIGNAL_RESULT] = 
        g_signal_new("result",
                     G_TYPE_FROM_CLASS(klass),
                     G_SIGNAL_RUN_LAST,
                     G_STRUCT_OFFSET(GstPocketSphinxClass, result),
                     NULL, NULL,
                     ps_marshal_VOID__STRING_STRING_INT,
                     G_TYPE_NONE,
                     3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT
            );

  gst_element_class_set_details_simple(gstelement_class,
    "PocketSphinx",
        "Filter/Audio",
        "Convert speech to text",
        "David Huggins-Daines <dhuggins@cs.cmu.edu>");

  gst_element_class_add_pad_template(gstelement_class,
                                       gst_static_pad_template_get(&sink_factory));
    gst_element_class_add_pad_template(gstelement_class,
                                       gst_static_pad_template_get(&src_factory));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_pocket_sphinx_init (GstPocketSphinx * filter)
{
    
  /* Create the hash table to store argument strings. */
  filter->arghash = g_hash_table_new(g_str_hash, g_str_equal);

  /* Parse default command-line options. */
  filter->config = cmd_ln_parse_r(NULL, ps_args(), default_argc, default_argv, FALSE);
  
  
  filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
  gst_pad_set_event_function (filter->sinkpad,
                              GST_DEBUG_FUNCPTR(gst_pocket_sphinx_sink_event));
  gst_pad_set_chain_function (filter->sinkpad,
                              GST_DEBUG_FUNCPTR(gst_pocket_sphinx_chain));
  
  gst_element_add_pad(GST_ELEMENT(filter), filter->sinkpad);
  gst_pad_use_fixed_caps(filter->sinkpad);
  
  filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
  gst_element_add_pad(GST_ELEMENT(filter), filter->srcpad);
  gst_pad_use_fixed_caps(filter->srcpad);

  /* Initialize time. */
  filter->last_result_time = 0;
  filter->last_result = NULL;

  /* Nbest size */
  filter->n_best_size = 10;
}

static void
gst_pocketsphinx_set_string(GstPocketSphinx *ps,
                            const gchar *key, const GValue *value)
{
    gchar *oldstr, *newstr;

    if (value != NULL)
        newstr = g_strdup(g_value_get_string(value));
    else
        newstr = NULL;
    if ((oldstr = g_hash_table_lookup(ps->arghash, key)))
        g_free(oldstr);
    cmd_ln_set_str_r(ps->config, key, newstr);
    g_hash_table_foreach(ps->arghash, (gpointer)key, newstr);
}

static void
gst_pocketsphinx_set_int(GstPocketSphinx *ps,
                         const gchar *key, const GValue *value)
{
    cmd_ln_set_int32_r(ps->config, key, g_value_get_int(value));
}

static void
gst_pocketsphinx_set_boolean(GstPocketSphinx *ps,
                             const gchar *key, const GValue *value)
{
    cmd_ln_set_boolean_r(ps->config, key, g_value_get_boolean(value));
}

static void
gst_pocketsphinx_set_float(GstPocketSphinx *ps,
                         const gchar *key, const GValue *value)
{
    cmd_ln_set_float_r(ps->config, key, g_value_get_float(value));
}

static void
gst_pocket_sphinx_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstPocketSphinx *ps = GST_POCKETSPHINX(object);

    switch (prop_id) {
    case PROP_CONFIGURED:
        if (ps->ps)
            ps_reinit(ps->ps, NULL);
        else
            ps->ps = ps_init(ps->config);
        break;
    case PROP_HMM_DIR:
        gst_pocketsphinx_set_string(ps, "-hmm", value);
        if (ps->ps) {
            /* Reinitialize the decoder with the new acoustic model. */
            ps_reinit(ps->ps, NULL);
        }
        break;
    case PROP_LM_FILE:
        /* FSG and LM are mutually exclusive. */
        gst_pocketsphinx_set_string(ps, "-fsg", NULL);
        gst_pocketsphinx_set_string(ps, "-lmctl", NULL);
        gst_pocketsphinx_set_string(ps, "-lm", value);
        if (ps->ps) {
            ngram_model_t *lm, *lmset;

            /* Switch to this new LM. */
            lm = ngram_model_read(ps->config,
                                  g_value_get_string(value),
                                  NGRAM_AUTO,
                                  ps_get_logmath(ps->ps));
            lmset = ps_get_lmset(ps->ps);
            ngram_model_set_add(lmset, lm, g_value_get_string(value),
                                1.0, TRUE);
            ps_update_lmset(ps->ps, lmset);
        }
        break;
    case PROP_LMCTL_FILE:
        /* FSG and LM are mutually exclusive. */
        gst_pocketsphinx_set_string(ps, "-fsg", NULL);
        gst_pocketsphinx_set_string(ps, "-lmctl", value);
        gst_pocketsphinx_set_string(ps, "-lm", NULL);
        if (ps->ps) {
            ngram_model_t *lmset;
            lmset = ngram_model_set_read(ps->config,
                                         g_value_get_string(value),
                                         ps_get_logmath(ps->ps));
            ps_update_lmset(ps->ps, lmset);
        }
        break;
    case PROP_LM_NAME:
        gst_pocketsphinx_set_string(ps, "-fsg", NULL);
        gst_pocketsphinx_set_string(ps, "-lmname", value);
        if (ps->ps) {
            ngram_model_t *lm, *lmset;

            lmset = ps_get_lmset(ps->ps);
            lm = ngram_model_set_select(lmset, g_value_get_string(value));
            ps_update_lmset(ps->ps, lmset);
        }

    case PROP_DICT_FILE:
        gst_pocketsphinx_set_string(ps, "-dict", value);
        if (ps->ps) {
            /* Reinitialize the decoder with the new dictionary. */
            ps_reinit(ps->ps, NULL);
        }
        break;
    case PROP_MLLR_FILE:
        gst_pocketsphinx_set_string(ps, "-mllr", value);
        if (ps->ps) {
            /* Reinitialize the decoder with the new MLLR transform. */
            ps_reinit(ps->ps, NULL);
        }
        break;
    case PROP_FSG_MODEL:
    {
        fsg_set_t *fsgs = ps_get_fsgset(ps->ps);
        
        if (fsgs == NULL)
            fsgs = ps_update_fsgset(ps->ps);
        
        if (fsgs) {
            fsg_model_t *fsg = g_value_get_pointer(value);

            fsg_set_remove_byname(fsgs, fsg_model_name(fsg));
            fsg_set_add(fsgs, fsg_model_name(fsg), fsg);
            fsg_set_select(fsgs, fsg_model_name(fsg));
        }
        break;
    }
    case PROP_FSG_FILE:
        /* FSG and LM are mutually exclusive */
        gst_pocketsphinx_set_string(ps, "-lm", NULL);
        gst_pocketsphinx_set_string(ps, "-fsg", value);

        if (ps->ps) {
            /* Switch to this new FSG. */
            fsg_model_t *fsg;
            fsg_set_t *fsgs = ps_get_fsgset(ps->ps);

            if (fsgs == NULL)
                fsgs = ps_update_fsgset(ps->ps);

            fsg = fsg_model_readfile(g_value_get_string(value),
                                     ps_get_logmath(ps->ps),
                                     cmd_ln_float32_r(ps->config, "-lw"));

            if (fsgs && fsg) {
                fsg_set_add(fsgs, fsg_model_name(fsg), fsg);
                fsg_set_select(fsgs, fsg_model_name(fsg));
            }
        }
        break;
    case PROP_FWDFLAT:
        gst_pocketsphinx_set_boolean(ps, "-fwdflat", value);
        break;
    case PROP_BESTPATH:
        gst_pocketsphinx_set_boolean(ps, "-bestpath", value);
        break;
    case PROP_LATDIR:
        if (ps->latdir)
            g_free(ps->latdir);
        ps->latdir = g_strdup(g_value_get_string(value));
        break;
    case PROP_NBEST_SIZE:
        ps->n_best_size = g_value_get_int(value);
        break;
    case PROP_MAXHMMPF:
        gst_pocketsphinx_set_int(ps, "-maxhmmpf", value);
        break;
    case PROP_MAXWPF:
        gst_pocketsphinx_set_int(ps, "-maxwpf", value);
        break;
    case PROP_BEAM:
        gst_pocketsphinx_set_float(ps, "-beam", value);
        break;
    case PROP_PBEAM:
        gst_pocketsphinx_set_float(ps, "-pbeam", value);
        break;
    case PROP_WBEAM:
        gst_pocketsphinx_set_float(ps, "-wbeam", value);
        break;
    case PROP_DSRATIO:
        gst_pocketsphinx_set_int(ps, "-ds", value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        return;
    }
}

static void
gst_pocket_sphinx_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
    GstPocketSphinx *ps = GST_POCKETSPHINX(object);

    switch (prop_id) {
    case PROP_DECODER:
        g_value_set_boxed(value, ps->ps);
        break;
    case PROP_CONFIGURED:
        g_value_set_boolean(value, ps->ps != NULL);
        break;
    case PROP_HMM_DIR:
        g_value_set_string(value, cmd_ln_str_r(ps->config, "-hmm"));
        break;
    case PROP_LM_FILE:
        g_value_set_string(value, cmd_ln_str_r(ps->config, "-lm"));
        break;
    case PROP_LMCTL_FILE:
        g_value_set_string(value, cmd_ln_str_r(ps->config, "-lmctl"));
        break;
    case PROP_LM_NAME:
        g_value_set_string(value, cmd_ln_str_r(ps->config, "-lmname"));
        break;
    case PROP_DICT_FILE:
        g_value_set_string(value, cmd_ln_str_r(ps->config, "-dict"));
        break;
    case PROP_MLLR_FILE:
        g_value_set_string(value, cmd_ln_str_r(ps->config, "-mllr"));
        break;
    case PROP_FSG_FILE:
        g_value_set_string(value, cmd_ln_str_r(ps->config, "-fsg"));
        break;
    case PROP_FWDFLAT:
        g_value_set_boolean(value, cmd_ln_boolean_r(ps->config, "-fwdflat"));
        break;
    case PROP_BESTPATH:
        g_value_set_boolean(value, cmd_ln_boolean_r(ps->config, "-bestpath"));
        break;
    case PROP_LATDIR:
        g_value_set_string(value, ps->latdir);
        break;
    case PROP_LATTICE: {
        ps_lattice_t *dag;

        if (ps->ps && (dag = ps_get_lattice(ps->ps)))
            g_value_set_boxed(value, dag);
        else
            g_value_set_boxed(value, NULL);
        break;
    }
    case PROP_MAXHMMPF:
        g_value_set_int(value, cmd_ln_int32_r(ps->config, "-maxhmmpf"));
        break;
    case PROP_MAXWPF:
        g_value_set_int(value, cmd_ln_int32_r(ps->config, "-maxwpf"));
        break;
    case PROP_BEAM:
        g_value_set_float(value, cmd_ln_float_r(ps->config, "-beam"));
        break;
    case PROP_PBEAM:
        g_value_set_float(value, cmd_ln_float_r(ps->config, "-pbeam"));
        break;
    case PROP_WBEAM:
        g_value_set_float(value, cmd_ln_float_r(ps->config, "-wbeam"));
        break;
    case PROP_DSRATIO:
        g_value_set_int(value, cmd_ln_int32_r(ps->config, "-ds"));
        break;
    case PROP_NBEST_SIZE:
        g_value_set_int(value, ps->n_best_size);
        break;
    case PROP_NBEST: {
        int i = 0, out_score = 0;
        GArray *arr;
        if (!ps->ps) {
            break;
        }
        arr = g_array_sized_new(FALSE, TRUE, sizeof(GValue), 1);
        ps_nbest_t *ps_nbest_list = ps_nbest(ps->ps, 0, -1, NULL, NULL);   
        if (ps_nbest_list) {
            ps_nbest_list = ps_nbest_next(ps_nbest_list);
            while ((i < ps->n_best_size) && (ps_nbest_list != NULL)) {
                GValue value1 = { 0 };
                g_value_init (&value1, G_TYPE_STRING);
                const char* hyp = ps_nbest_hyp(ps_nbest_list, &out_score);
                g_value_set_string(&value1, hyp);
                g_array_append_val (arr, value1);
                ps_nbest_list = ps_nbest_next(ps_nbest_list);
                i++;
            }
            if (ps_nbest_list) {
                ps_nbest_free(ps_nbest_list);
            }
        }
        g_value_set_boxed (value, arr);
        break;
    }
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

/* GstElement vmethod implementations */

/* this function handles sink events */
static gboolean
gst_pocket_sphinx_sink_event (GstPad * pad, GstObject * parent, GstEvent * event)
{
    GstPocketSphinx *ps;

    ps = GST_POCKETSPHINX(parent);

    /* Pick out VAD events. */
    switch (event->type) {
    case GST_EVENT_SEGMENT:
        /* Initialize the decoder once the audio starts, if it's not
         * there yet. */
        if (ps->ps == NULL) {
            ps->ps = ps_init(ps->config);
            if (ps->ps == NULL) {
                GST_ELEMENT_ERROR(GST_ELEMENT(ps), LIBRARY, INIT,
                                  ("Failed to initialize PocketSphinx"),
                                  ("Failed to initialize PocketSphinx"));
                return FALSE;
            }
        }
        return gst_pad_event_default(pad, parent, event);
    case GST_EVENT_VADER_START:
        ps->listening = TRUE;
        ps_start_utt(ps->ps, NULL);
        /* Forward this event. */
        return gst_pad_event_default(pad, parent, event);
    case GST_EVENT_EOS:
    case GST_EVENT_VADER_STOP: {
        GstBuffer *buffer;
        int32 score;
        char const *hyp;
        char const *uttid;

        hyp = NULL;
        if (ps->listening) {
            ps->listening = FALSE;
            ps_end_utt(ps->ps);
            hyp = ps_get_hyp(ps->ps, &score, &uttid);
            /* Dump the lattice if requested. */
            if (ps->latdir) {
                char *latfile = string_join(ps->latdir, "/", uttid, ".lat", NULL);
                ps_lattice_t *dag;

                if ((dag = ps_get_lattice(ps->ps)))
                    ps_lattice_write(dag, latfile);
                ckd_free(latfile);
            }
        }
        if (hyp) {
            /* Emit a signal for applications. */
            g_signal_emit(ps, gst_pocketsphinx_signals[SIGNAL_RESULT],
                          0, hyp, uttid, score);
            /* Forward this result in a buffer. */
            buffer = gst_buffer_new_and_alloc(strlen(hyp) + 2);
            
            GstMapInfo mapInfo;
            gst_buffer_map( buffer, &mapInfo, GST_MAP_READ | GST_MAP_WRITE );
            
            strcpy((char *)mapInfo.data, hyp);
            mapInfo.data[strlen(hyp)] = '\n';
            mapInfo.data[strlen(hyp)+1] = '\0';
            GST_BUFFER_DTS(buffer) = GST_EVENT_TIMESTAMP(event);
//            gst_buffer_set_caps(buffer, GST_PAD_CAPS(ps->srcpad));
            
            gst_buffer_unmap(buffer, &mapInfo);
            
            gst_pad_push(ps->srcpad, buffer);
        }

        /* Forward this event. */
        return gst_pad_event_default(pad, parent, event);
    }
    default:
        /* Don't bother with other events. */
        return gst_pad_event_default(pad, parent, event);
    }
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_pocket_sphinx_chain (GstPad * pad, GstObject * parent, GstBuffer * buf)
{
    GstPocketSphinx *ps;

    ps = GST_POCKETSPHINX(parent);

    /* Start an utterance for the first buffer we get (i.e. we assume
     * that the VADER is "leaky") */
    if (!ps->listening) {
        ps->listening = TRUE;
        ps_start_utt(ps->ps, NULL);
    }
    GstMapInfo mapInfo;
    gst_buffer_map( buf, &mapInfo, GST_MAP_READ | GST_MAP_WRITE );
    ps_process_raw(ps->ps,
                   (short *)mapInfo.data,
                   mapInfo.size / sizeof(short),
                   FALSE, FALSE);

    /* Get a partial result every now and then, see if it is different. */
    if (ps->last_result_time == 0
        /* Check every 100 milliseconds. */
        || (GST_BUFFER_PTS(buf) - ps->last_result_time) > 100*10*1000) {
        int32 score;
        char const *hyp;
        char const *uttid;

        hyp = ps_get_hyp(ps->ps, &score, &uttid);
        ps->last_result_time = GST_BUFFER_PTS(buf);
        if (hyp && strlen(hyp) > 0) {
            if (ps->last_result == NULL || 0 != strcmp(ps->last_result, hyp)) {
                g_free(ps->last_result);
                ps->last_result = g_strdup(hyp);
                /* Emit a signal for applications. */
                g_signal_emit(ps, gst_pocketsphinx_signals[SIGNAL_PARTIAL_RESULT],
                              0, hyp, uttid, score);
            }
        }
    }
    gst_buffer_unmap( buf, &mapInfo );
    gst_buffer_unref(buf);
    return GST_FLOW_OK;
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
pocketsphinx_init (GstPlugin * speechrecognition)
{
  /* debug category for fltering log messages
   *
   * exchange the string 'Template pocketsphinx' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_pocket_sphinx_debug, "pocketsphinx",
      0, "Pocketsphinx");
  /*GST_DEBUG_CATEGORY_INIT (gst_vader_debug, "vader",
      0, "Vader");*/
  
  if (!gst_element_register(speechrecognition, "pocketsphinx",
                              GST_RANK_NONE, GST_TYPE_POCKETSPHINX))
        return FALSE;
  if (!gst_element_register(speechrecognition, "googlespeech",
                              GST_RANK_NONE, GST_TYPE_GOOGLESPEECH))
        return FALSE;
  if (!gst_element_register(speechrecognition, "vader",
                              GST_RANK_NONE, GST_TYPE_VADER))
        return FALSE;
    
    return TRUE;
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "speechrecognition"
#endif

/* gstreamer looks for this structure to register pocketsphinxs
 *
 * exchange the string 'Template pocketsphinx' with your pocketsphinx description
 */
GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  speechrecognition,
                  "Speech recognition plugin",
                  pocketsphinx_init, VERSION,
#if (GST_VERSION_MINOR == 10 && GST_VERSION_MICRO < 15) /* Nokia's bogus old GStreamer */
                  "LGPL",
#else
                  "BSD",
#endif
                  "PocketSphinx", "http://cmusphinx.sourceforge.net/")
