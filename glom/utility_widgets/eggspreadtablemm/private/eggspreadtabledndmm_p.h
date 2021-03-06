#ifndef _EGG_SPREADTABLE_DND_P_H
#define _EGG_SPREADTABLE_DND_P_H


#include <glom/utility_widgets/eggspreadtablemm/private/eggspreadtablemm_p.h>

#include <glibmm/class.h>

namespace Egg
{

class SpreadTableDnd_Class : public Glib::Class
{
public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  typedef SpreadTableDnd CppObjectType;
  typedef EggSpreadTableDnd BaseObjectType;
  typedef EggSpreadTableDndClass BaseClassType;
  typedef SpreadTable_Class CppClassParent;
  typedef EggSpreadTableClass BaseClassParent;

  friend class SpreadTableDnd;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

  const Glib::Class& init();


  static void class_init_function(void* g_class, void* class_data);

  static Glib::ObjectBase* wrap_new(GObject*);

protected:

  //Callbacks (default signal handlers):
  //These will call the *_impl member methods, which will then call the existing default signal callbacks, if any.
  //You could prevent the original default signal handlers being called by overriding the *_impl method.

  static gboolean widget_drop_possible_callback(EggSpreadTableDnd* self, GtkWidget* p0, gboolean* drop_possible);

  //Callbacks (virtual functions):
};


} // namespace Egg


#endif /* _EGG_SPREADTABLE_P_H */
