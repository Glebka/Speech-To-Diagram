#!/usr/bin/python

import sys

from gaphor import Application
import gaphor.UML as UML
from gaphor.diagram import items
import gaphor.diagram as diagram
from cairo import Matrix

Application.essential_services.append('main_window')

Application.init()

main_window = Application.get_service('main_window')

main_window.open()

file_manager = Application.get_service('file_manager')
element_factory = Application.get_service('element_factory')
element_factory.flush()


model = element_factory.create(UML.Package)
model.name = 'New model'

obj = element_factory.create(UML.Class)
obj.package = model

diagram = element_factory.create(UML.Diagram)
diagram.package = model
diagram.name= 'main'
klass=diagram.create(items.ClassItem, subject=obj)

klass.subject.name='testttt'
attr = element_factory.create(UML.Property)
attr.name = 4 * 'x' # about 44 pixels
klass.subject.ownedAttribute = attr
klass.matrix *= Matrix(xx = 1.0, yx = 0.0, xy = 0.0, yy = 1.0, x0 = 100, y0 = 200)
klass.request_update()
element_factory.notify_model()

#diagram.canvas.update()
#main_window.get_current_diagram_tab().get_view().queue_draw_refresh()

#item = diagram.create(items.ClassItem, subject=obj)

Application.run()