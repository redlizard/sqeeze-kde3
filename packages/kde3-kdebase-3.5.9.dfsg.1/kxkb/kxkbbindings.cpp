#ifndef NOSLOTS
# define DEF( name, key3, key4, fnSlot ) \
   keys->insert( name, i18n(name), QString::null, key3, key4, this, SLOT(fnSlot) )
#else
# define DEF( name, key3, key4, fnSlot ) \
   keys->insert( name, i18n(name), QString::null, key3, key4 )
#endif

	keys->insert( "Program:kxkb", i18n("Keyboard") );
	DEF( I18N_NOOP("Switch to Next Keyboard Layout"), ALT+CTRL+Qt::Key_K, KKey::QtWIN+CTRL+Qt::Key_K, toggled() );

#undef DEF
