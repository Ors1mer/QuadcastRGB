/* locale_macros.h
 * Stuff for proper work of gettext
 * in order to achieve multi-language support.
 *
 * This is a header without the source file (because it isn't needed).
 */
#include <locale.h>
#include <libintl.h>

/* Constants */
#define LOCALEBASEDIR "../lang"
#define TEXTDOMAIN "hyperrgb"

/* Macros */
#define _(STR) (STR) /* the string is language-dependent */
#define N_(STR) (STR) /* it isn't */
