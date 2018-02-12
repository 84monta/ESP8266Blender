import bpy
import requests
import time


class ModalTimerOperator(bpy.types.Operator):
    Previous_direction = "TOP"
    Prev_time   = 0
    counter = 0
    """Operator which runs its self from a timer"""
    bl_idname = "wm.modal_timer_operator"
    bl_label = "Modal Timer Operator"
    
    
    
    _timer = None

    def modal(self, context, event):
        if event.type in {'RIGHTMOUSE', 'ESC'}:
            self.cancel(context)
            return {'CANCELLED'}

        if event.type == 'TIMER':
            if self.counter < 100:
                self.counter = self.counter + 1
                exit

            ret = requests.get("http://192.168.100.1/direction")
            for area in bpy.context.screen.areas:
                if area.type == 'VIEW_3D':
                    if self.Previous_direction != ret.text:
                        self.Previous_direction = ret.text
                        override = bpy.context.copy()
                        override['area'] = area
                        bpy.ops.view3d.viewnumpad(override, type = ret.text)
                    

        return {'PASS_THROUGH'}

    def execute(self, context):
        wm = context.window_manager
        self._timer = wm.event_timer_add(0.3, context.window)
        wm.modal_handler_add(self)
        return {'RUNNING_MODAL'}

    def cancel(self, context):
        wm = context.window_manager
        wm.event_timer_remove(self._timer)


def register():
    bpy.utils.register_class(ModalTimerOperator)


def unregister():
    bpy.utils.unregister_class(ModalTimerOperator)


if __name__ == "__main__":
    register()

    # test call
    bpy.ops.wm.modal_timer_operator()
