import gi
# 
gi.require_version('GIRepository', '2.0')
from gi.repository import GIRepository
girepo = GIRepository.Repository.get_default()
girepo.prepend_search_path('/home/la/git/srain/prefix/lib/girepository-1.0')
girepo.prepend_library_path('/home/la/git/srain/prefix/lib')

gi.require_version('GLib', '2.0')
gi.require_version('Gio', '2.0')
gi.require_version('GObject', '2.0')
gi.require_version('Srn', '1.0')

from gi.repository import GLib
from gi.repository import Gio
from gi.repository import GObject
from gi.repository import Srn

print('>>>>>>>>>>>>>>>>>>>')

class StgMessenger(GObject.GObject, Srn.Messenger):
    def __init__(self) -> None:
        GObject.GObject.__init__(self)

    @GObject.Property(type=str)
    def name(self) -> str:
        return 'stg'

    @GObject.Property(type=str)
    def pretty_name(self) -> str:
        return 'Telegram Messenger'

    @GObject.Property(type=int)
    def version(self) -> int:
        return 1

    @GObject.Property(type=str)
    def schemas(self) -> str:
        return 'tg'

Gio.IOExtensionPoint.implement(
    Srn.MESSENGER_EXTENSION_POINT_NAME,
    StgMessenger.__gtype__,
    'stg',
    10)
