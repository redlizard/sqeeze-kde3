INCLUDES = \
    -I$(srcdir) \
    -I$(srcdir)/../sdk \
    -I$(srcdir)/../core \
    -I$(srcdir)/../kritacolor \
    $(KOFFICE_INCLUDES) \
    $(KOPAINTER_INCLUDES) \
    $(KDE_INCLUDES) \
    $(all_includes)

lib_LTLIBRARIES = libkritaui.la
libkritaui_la_LDFLAGS =  -version-info 1:0:0 -no-undefined $(all_libraries) 
libkritaui_la_LIBADD = ../sdk/libkritasdk.la ../core/libkritaimage.la ../kritacolor/libkritacolor.la \
    $(LCMS_LIBS) $(LIB_KOFFICEUI) $(LIB_KOPAINTER) $(LIB_KOPALETTE) $(LIB_XINPUTEXT) $(GLLIB)

libkritaui_la_SOURCES = kis_import_catcher.cc kis_histogram_view.cc imageviewer.cc kcurve.cc \
				kis_autobrush.cc kis_autogradient.cc kis_boundary_painter.cc kis_brush_chooser.cc \
				kis_canvas.cc kis_canvas_painter.cc kis_clipboard.cc kis_cmb_composite.cc \
				kis_cmb_idlist.cc kis_color_cup.cc kis_config.cc kis_controlframe.cc kis_cursor.cc \
				kis_dlg_apply_profile.cc kis_dlg_image_properties.cc kis_dlg_layer_properties.cc \
				kis_dlg_new_layer.cc kis_dlg_preferences.cc kis_doc.cc kis_doc_iface.cc kis_doc_iface.skel \
				kis_double_widget.cc kis_factory.cc kis_filter_manager.cc kis_gradient_chooser.cc \
				kis_gradient_slider_widget.cc kis_icon_item.cc kis_iconwidget.cc kis_int_spinbox.cc \
	kis_itemchooser.cc 			kis_label_cursor_pos.cc kis_label_progress.cc kis_label_zoom.cc \
	kis_layerbox.cc 			kis_layerlist.cc kis_multi_bool_filter_widget.cc \
	kis_multi_double_filter_widget.cc 			kis_multi_integer_filter_widget.cc kis_opengl_canvas.cc \
	kis_opengl_canvas_painter.cc 			kis_opengl_image_context.cc kis_paintop_box.cc kis_palette_view.cc \
	kis_palette_widget.cc 			kis_part_layer.cc kis_pattern_chooser.cc kis_previewdialog.cc \
	kis_previewwidget.cc 			kis_qpaintdevice_canvas.cc kis_qpaintdevice_canvas_painter.cc \
	kis_resource_mediator.cc 			kis_resourceserver.cc kis_ruler.cc kis_selection_manager.cc \
	kis_selection_options.cc 			kis_text_brush.cc kis_tool.cc kis_tool_dummy.cc kis_tool_freehand.cc \
	kis_tool_manager.cc 			kis_tool_non_paint.cc kis_tool_paint.cc kis_tool_registry.cc \
	kis_tool_shape.cc 			kis_birdeye_box.cc kis_view.cc kis_view_iface.cc kis_custom_brush.cc \
	kis_custom_palette.cc 			kis_custom_pattern.cc kis_custom_image_widget.cc kis_view_iface.skel \
	kobirdeyepanel.cpp 			kis_matrix_widget.ui kis_previewwidgetbase.ui layerlist.cpp \
	wdgapplyprofile.ui 			wdgautobrush.ui wdgautogradient.ui wdgbirdeye.ui wdgcolorsettings.ui \
				wdgdisplaysettings.ui wdggeneralsettings.ui wdglayerproperties.ui wdglayerbox.ui \
				wdgnewimage.ui wdgperformancesettings.ui wdgselectionoptions.ui wdgshapeoptions.ui \
				wdgpressuresettings.ui wdgcustombrush.ui wdgcustompalette.ui wdgcustompattern.ui \
				wdgtextbrush.ui kis_dlg_adjustment_layer.cc kis_filters_listview.cc \
				wdgpalettechooser.ui wdggridsettings.ui kis_grid_manager.cpp 			wdgtabletdevicesettings.ui \
	wdgtabletsettings.ui kis_input_device.cc 			kis_part_layer_handler.cc \
	kis_dlg_adj_layer_props.cc squeezedcombobox.cpp kis_perspective_grid_manager.cpp \
	kis_grid_drawer.cpp

noinst_HEADERS = kis_aboutdata.h imageviewer.h layerlist.h kcurve.h \
			kis_autobrush.h kis_autogradient.h kis_birdeye_box.h kis_brush_chooser.h \
			kis_button_press_event.h kis_canvas.h 		kis_clipboard.h kis_controlframe.h \
	kis_dlg_apply_profile.h kis_dlg_image_properties.h 		kis_dlg_layer_properties.h \
	kis_dlg_new_layer.h kis_dlg_preferences.h 		kis_event.h kis_factory.h \
	kis_label_cursor_pos.h kis_label_progress.h 		kis_part_layer.h kis_pattern_chooser.h \
	kis_resource_mediator.h kis_resourceserver.h 		kis_ruler.h kis_selection_manager.h \
	kis_selection_options.h kis_view_iface.h 		kis_custom_brush.h kis_custom_pattern.h \
	kis_custom_image_widget.h 		kis_dlg_adjustment_layer.h kis_grid_manager.h \
	kis_dlg_adj_layer_props.h squeezedcombobox.h kis_perspective_grid_manager.h

include_HEADERS = kis_button_event.h kis_button_release_event.h kis_canvas_painter.h kis_cmb_composite.h kis_cmb_idlist.h kis_color_cup.h kis_config.h \
		kis_cursor.h kis_doc.h kis_doc_iface.h \
		kis_double_click_event.h kis_double_widget.h kis_event.h kis_filter_manager.h \
		kis_gradient_chooser.h kis_gradient_slider_widget.h kis_histogram_view.h kis_icon_item.h \
		kis_iconwidget.h kis_itemchooser.h \
		kis_label_zoom.h kis_int_spinbox.h kis_layerbox.h kis_layerlist.h kis_matrix_widget.ui.h kis_move_event.h \
		kis_multi_bool_filter_widget.h kis_multi_double_filter_widget.h kis_multi_integer_filter_widget.h \
		kis_paintop_box.h kis_palette_widget.h \
		kis_previewdialog.h kis_previewwidget.h \
		kis_tool_controller.h kis_tool.h kis_tool_non_paint.h kis_tool_paint.h kis_tool_freehand.h  \
		kis_tool_dummy.h kis_tool_manager.h kis_tool_types.h  kis_tool_registry.h kis_view.h \
		kis_tool_factory.h \
		kobirdeyepanel.h \
		kis_filters_listview.h kis_input_device.h kis_opengl_image_context.h

METASOURCES = AUTO

KDE_OPTIONS = nofinal

