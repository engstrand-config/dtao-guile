#pragma once

SCM config;

/* Config variable definitions. */
/* These will be automatically set from the guile config. */
static uint32_t height                  = 0;
static int exclusive                    = -1;
static int adjustwidth                  = 0;
static int isbottom                     = 0;
static int usewmcolorscheme             = 0;
static int borderpx                     = 0;
static char *fontstr                    = "";

/* positioning */
static enum align titlealign = ALIGN_L, subalign = ALIGN_R;
static int layer = ZWLR_LAYER_SHELL_V1_LAYER_TOP;;
static int anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
                    ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
		    ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;

/* blocks */
static Block *titleblocks               = NULL;
static Block *subblocks                 = NULL;
static unsigned int numsubblocks        = 0;
static unsigned int numtitleblocks      = 0;

/* default colors */
static pixman_color_t
	bordercolor = {
		.red = 0xffff,
		.green = 0x0000,
		.blue = 0x0000,
		.alpha = 0xffff,
	},
	bgcolor = {
		.red = 0x1111,
		.green = 0x1111,
		.blue = 0x1111,
		.alpha = 0x0000,
	},
	fgcolor = {
		.red = 0xb3b3,
		.green = 0xb3b3,
		.blue = 0xb3b3,
		.alpha = 0xffff,
	};

static inline void
dscm_parse_block(unsigned int index, SCM block, void *data)
{
        ((Block*)data)[index] = (Block){
                .signal = dscm_alist_get_int(block, "signal"),
                .interval = dscm_alist_get_int(block, "interval"),
                .click = dscm_alist_get_proc_pointer(block, "click"),
                .render = dscm_alist_get_proc_pointer(block, "render")
        };
}

static inline void
dscm_config_parse(char *configfile)
{
        SCM eval, barlayer, twalign, swalign;

        scm_c_primitive_load(configfile);
        config = dscm_get_variable("config");
        if (scm_is_null(config))
                BARF("invalid config");

        fontstr = dscm_alist_get_string(config, "font");
        height = dscm_alist_get_int(config, "height");
        borderpx = dscm_alist_get_int(config, "border-px");
        exclusive = dscm_alist_get_int(config, "exclusive");
        isbottom = dscm_alist_get_int(config, "bottom");
        adjustwidth = dscm_alist_get_int(config, "adjust-width");
        usewmcolorscheme = dscm_alist_get_int(config, "use-dwl-guile-colorscheme");

        if (isbottom)
                anchor ^= ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
                          ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM;
        if (!usewmcolorscheme) {
                char *bgstr, *fgstr, *borderstr;
                bgstr = dscm_alist_get_string(config, "background-color");
                fgstr = dscm_alist_get_string(config, "foreground-color");
                borderstr = dscm_alist_get_string(config, "border-color");
                parse_color(bgstr, &bgcolor);
                parse_color(fgstr, &fgcolor);
                parse_color(borderstr, &bordercolor);
                free(bgstr);
                free(fgstr);
                free(borderstr);
        }

        barlayer = dscm_alist_get(config, "layer");
        eval = scm_primitive_eval(barlayer);
        layer = scm_to_int(eval);

        twalign = dscm_alist_get(config, "title-align");
        eval = scm_primitive_eval(twalign);
        titlealign = (enum align)scm_to_int(eval);

        swalign = dscm_alist_get(config, "sub-align");
        eval = scm_primitive_eval(swalign);
        subalign = (enum align)scm_to_int(eval);

        titleblocks = dscm_iterate_list(dscm_alist_get(config, "title-blocks"),
                sizeof(Block), &dscm_parse_block, &numtitleblocks);
        subblocks = dscm_iterate_list(dscm_alist_get(config, "sub-blocks"),
                sizeof(Block), &dscm_parse_block, &numsubblocks);
}

static inline void
dscm_config_cleanup()
{
        free(titleblocks);
        free(subblocks);
        free(fontstr);
}
