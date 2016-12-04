/* main.c: launching the program
   Copyright 2016 The Tectonic Project
   Licensed under the MIT License.
*/

#include <tectonic/tectonic.h>
#include <tectonic/internals.h>
#include <tectonic/xetexd.h>
#include <tectonic/XeTeX_ext.h>

#include <signal.h>


#define ETEX_VERSION "2.6"
#define XETEX_VERSION "0.99996"
#define BANNER "This is Tectonic, Version 3.14159265-" ETEX_VERSION "-" XETEX_VERSION
/* these only show up in the -version output: */
#define COPYRIGHT_HOLDER "The Tectonic Project"
#define AUTHOR "The Tectonic Project"
#define BUG_ADDRESS "https://github.com/pkgw/tectonic/issues"
#define SYNCTEX_NO_OPTION INT_MAX


const_string help_lines[] = {
    "Usage: tectonic-compat [OPTION]... [TEXNAME[.tex]]",
    "  Process TEXNAME, usually creating TEXNAME.pdf.",
    "",
    "-etex                   enable e-TeX extensions",
    "-file-line-error        enable file:line:error style messages",
    "-fmt=FMTNAME            use FMTNAME instead of program name or a %& line",
    "-halt-on-error          stop processing at the first error",
    "-ini                    be xeinitex, for dumping formats; this is implicitly",
    "                          true if the program name is `xeinitex'",
    "-interaction=STRING     set interaction mode (STRING=batchmode/nonstopmode/",
    "                          scrollmode/errorstopmode)",
    "-mltex                  enable MLTeX extensions such as \\charsubdef",
    "-output-comment=STRING  use STRING for XDV file comment instead of date",
    "-output-driver=CMD      use CMD as the XDV-to-PDF driver instead of xdvipdfmx",
    "-no-pdf                 generate XDV (extended DVI) output rather than PDF",
    "-papersize=STRING       set PDF media size to STRING",
    "-src-specials           insert source specials into the XDV file",
    "-src-specials=WHERE     insert source specials in certain places of",
    "                          the XDV file. WHERE is a comma-separated value",
    "                          list: cr display hbox math par parend vbox",
    "-synctex=NUMBER         generate SyncTeX data for previewers if nonzero",
    "-8bit                   make all characters printable, don't use ^^X sequences",
    "-help                   display this help and exit",
    "-version                output version information and exit",
    NULL
};

static void parse_src_specials_option (const_string);
static string parse_options (int, string *);


static void
usagehelp (const_string *message, const_string bug_email)
{
    if (!bug_email)
        bug_email = "tex-k@tug.org";
    while (*message) {
        printf("%s\n", *message);
        ++message;
    }
    printf("\nEmail bug reports to %s.\n", bug_email);
    exit(0);
}

/* formerly printversion.c */

const char *version_string = " (Tectonic)";

/* We're passed in the original WEB banner string, which has the form
This is PROGRAM, Version VERSION-NUMBER
   We parse the PROGRAM and VERSION-NUMBER out of this.
   
   If COPYRIGHT_HOLDER is specified and AUTHOR isn't, then use the
   former for the latter.  If AUTHOR is specified and COPYRIGHT_HOLDER
   isn't, it means the original program is public domain.
   
   Maybe I should have just done it all inline in each individual
   program, but tangle doesn't allow multiline string constants ...  */

void
printversionandexit (const_string banner,
                     const_string copyright_holder,  
                     const_string author,
                     const_string extra_info)
{
  string prog_name;
  unsigned len;
  const_string prog_name_end = strchr (banner, ',');
  const_string prog_version = strrchr (banner, ' ');
  assert (prog_name_end && prog_version);
  prog_version++;
  
  len = prog_name_end - banner - sizeof ("This is");
  prog_name = xmalloc (len + 1);
  strncpy (prog_name, banner + sizeof ("This is"), len);
  prog_name[len] = 0;

  /* The Web2c version string starts with a space.  */
  printf ("%s %s%s\n", prog_name, prog_version, version_string);

  if (copyright_holder) {
    printf ("Copyright 2016 %s.\n", copyright_holder);
    if (!author)
      author = copyright_holder;
  }

  puts ("There is NO warranty.  Redistribution of this software is");
  fputs ("covered by the terms of ", stdout);
  printf ("both the %s copyright and\n", prog_name);
  puts ("the Lesser GNU General Public License.");
  puts ("For more information about these matters, see the file");
  printf ("named COPYING and the %s source.\n", prog_name);
  printf ("Primary author of %s: %s.\n", prog_name, author);

  if (extra_info)
    fputs (extra_info, stdout);

  exit (0);
}


static RETSIGTYPE
catch_interrupt (int arg)
{
    interrupt = 1;
    (void) signal (SIGINT, catch_interrupt);
}


int
main (int argc, string *argv)
{
    string input_file_name;
    const_string with_ext = NULL;
    unsigned int name_len;

    input_file_name = parse_options (argc, argv);

    if (!in_initex_mode && mltex_p)
	fprintf(stderr, "-mltex only works with -ini\n");

    /* Make a nice name for the format ("dump") file. */

    if (!dump_name)
	dump_name = "xelatex";

    name_len = strlen (dump_name);

    if (name_len > 4 && FILESTRCASEEQ (dump_name + name_len - 4, ".fmt"))
	with_ext = dump_name;
    else
	with_ext = concat (dump_name, ".fmt");

    TEX_format_default = concat (" ", with_ext); /* adjust array for Pascal */
    format_default_length = strlen (TEX_format_default + 1);

    /* Signal handling done with global variables. C'est la vie. */

    signal (SIGINT, catch_interrupt);

    /* Ready to jump into the engine. */

    return main_body (input_file_name);
}


#define ARGUMENT_IS(a) STREQ (long_options[option_index].name, a)

static struct option long_options[] = {
    { "fmt",                       1, 0, 0 },
    { "help",                      0, 0, 0 },
    { "ini",                       0, &in_initex_mode, 1 },
    { "interaction",               1, 0, 0 },
    { "halt-on-error",             0, &halt_on_error_p, 1 },
    { "version",                   0, 0, 0 },
    { "mltex",                     0, &mltex_p, 1 },
    { "etex",                      0, &etex_p, 1 },
    { "output-comment",            1, 0, 0 },
    { "debug-format",              0, &debug_format_file, 1 },
    { "src-specials",              2, 0, 0 },
    { "synctex",                   1, 0, 0 },
    { "file-line-error",           0, &file_line_error_style_p, 1 },
    { "8bit",                      0, &eight_bit_p, 1 },
    { "no-pdf",                    0, &no_pdf_output, 1 },
    { "output-driver",             1, 0, 0 },
    { "papersize",                 1, 0, 0 },
    { 0, 0, 0, 0 } };


static string
parse_options (int argc, string *argv)
{
    int g;   /* `getopt' return code.  */
    int option_index;

    /* Not sure why these are the only ones that get custom initializations. */
    interaction_option = 4;
    synctexoption = SYNCTEX_NO_OPTION;

    for (;;) {
	g = getopt_long_only (argc, argv, "+", long_options, &option_index);

	if (g == -1) /* End of arguments, exit the loop.  */
	    break;

	if (g == '?') { /* Unknown option.  */
	    /* FIXME: usage (argv[0]); replaced by continue. */
	    continue;
	}

	assert (g == 0); /* We have no short option names.  */

	if (ARGUMENT_IS ("papersize")) {
	    papersize = optarg;
	} else if (ARGUMENT_IS ("output-driver")) {
	    outputdriver = optarg;
	} else if (ARGUMENT_IS ("fmt")) {
	    dump_name = optarg;
	} else if (ARGUMENT_IS ("output-comment")) {
	    unsigned len = strlen (optarg);
	    if (len < 256) {
		output_comment = optarg;
	    } else {
		WARNING2 ("Comment truncated to 255 characters from %d. (%s)",
			  len, optarg);
		output_comment = xmalloc (256);
		strncpy (output_comment, optarg, 255);
		output_comment[255] = 0;
	    }
	} else if (ARGUMENT_IS ("src-specials")) {
	    if (optarg == NULL) {
		/* "auto" mode */
		insert_src_special_every_par = true;
		insert_src_special_auto = true;
		src_specials_p = true;
	    } else {
		parse_src_specials_option(optarg);
	    }
	} else if (ARGUMENT_IS ("interaction")) {
	    /* These numbers match @d's in *.ch */
	    if (STREQ (optarg, "batchmode")) {
		interaction_option = 0;
	    } else if (STREQ (optarg, "nonstopmode")) {
		interaction_option = 1;
	    } else if (STREQ (optarg, "scrollmode")) {
		interaction_option = 2;
	    } else if (STREQ (optarg, "errorstopmode")) {
		interaction_option = 3;
	    } else {
		WARNING1 ("Ignoring unknown argument `%s' to --interaction", optarg);
	    }
	} else if (ARGUMENT_IS ("help")) {
	    usagehelp (help_lines, BUG_ADDRESS);
	} else if (ARGUMENT_IS ("synctex")) {
	    /* Synchronize TeXnology: catching the command line option as a long  */
	    synctexoption = (int) strtol(optarg, NULL, 0);
	} else if (ARGUMENT_IS ("version")) {
	    char *versions;
	    initversionstring(&versions);
	    printversionandexit (BANNER, COPYRIGHT_HOLDER, AUTHOR, versions);
	} /* Else it was a flag; getopt has already done the assignment.  */
    }

    if (file_line_error_style_p < 0)
	file_line_error_style_p = 0;

    /* Now deal with the non-option arguments. */

    if (optind != argc - 1) {
	fprintf (stderr, "error: expected exactly one non-option argument\n");
	exit (1);
    }

    return argv[optind];
}


static void
parse_src_specials_option (const_string opt_list)
{
    char * toklist = xstrdup(opt_list);
    char * tok;

    insert_src_special_auto = false;
    tok = strtok (toklist, ", ");

    while (tok) {
	if (strcmp (tok, "everypar") == 0
	    || strcmp (tok, "par") == 0
	    || strcmp (tok, "auto") == 0) {
	    insert_src_special_auto = true;
	    insert_src_special_every_par = true;
	} else if (strcmp (tok, "everyparend") == 0
		   || strcmp (tok, "parend") == 0)
	    insert_src_special_every_parend = true;
	else if (strcmp (tok, "everycr") == 0
		 || strcmp (tok, "cr") == 0)
	    insert_src_special_every_cr = true;
	else if (strcmp (tok, "everymath") == 0
		 || strcmp (tok, "math") == 0)
	    insert_src_special_every_math = true;
	else if (strcmp (tok, "everyhbox") == 0
		 || strcmp (tok, "hbox") == 0)
	    insert_src_special_every_hbox = true;
	else if (strcmp (tok, "everyvbox") == 0
		 || strcmp (tok, "vbox") == 0)
	    insert_src_special_every_vbox = true;
	else if (strcmp (tok, "everydisplay") == 0
		 || strcmp (tok, "display") == 0)
	    insert_src_special_every_display = true;
	else if (strcmp (tok, "none") == 0) {
	    /* This one allows to reset an option that could appear in texmf.cnf */
	    insert_src_special_auto = insert_src_special_every_par =
		insert_src_special_every_parend = insert_src_special_every_cr =
		insert_src_special_every_math =  insert_src_special_every_hbox =
		insert_src_special_every_vbox = insert_src_special_every_display = false;
	} else {
	    WARNING1 ("Ignoring unknown argument `%s' to --src-specials", tok);
	}

	tok = strtok(0, ", ");
    }

    free(toklist);
    src_specials_p = insert_src_special_auto | insert_src_special_every_par |
	insert_src_special_every_parend | insert_src_special_every_cr |
	insert_src_special_every_math |  insert_src_special_every_hbox |
	insert_src_special_every_vbox | insert_src_special_every_display;
}
