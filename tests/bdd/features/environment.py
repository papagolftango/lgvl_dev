import os
from support.bridge import get_bridge


def before_all(context):
    # Bridge configuration via env vars
    # BRIDGE_MODE = mock|http|serial
    # BRIDGE_URL = http://host:port
    # SERIAL_PORT, SERIAL_BAUD
    context.bridge = get_bridge(
        mode=os.getenv("BRIDGE_MODE", "mock"),
        url=os.getenv("BRIDGE_URL"),
        serial_port=os.getenv("SERIAL_PORT"),
        serial_baud=int(os.getenv("SERIAL_BAUD", "115200")),
    )

    context.state = {}


def after_all(context):
    if hasattr(context, "bridge") and context.bridge:
        try:
            context.bridge.close()
        except Exception:
            pass
