from __future__ import annotations
import json
import time
from dataclasses import dataclass
from typing import Any, Dict, Optional

try:
    import requests
except Exception:  # pragma: no cover
    requests = None

try:
    import serial
except Exception:  # pragma: no cover
    serial = None


class Bridge:
    def close(self):
        pass

    # App manager
    def get_active_app(self) -> str:
        raise NotImplementedError

    def set_active_app(self, name: str) -> None:
        raise NotImplementedError

    # Inputs
    def tap(self) -> None:
        raise NotImplementedError

    def rotate(self, direction: str) -> None:
        raise NotImplementedError

    # Power/time
    def set_inactivity_timeout(self, seconds: int) -> None:
        raise NotImplementedError

    def get_power_state(self) -> str:
        raise NotImplementedError

    def set_power_state(self, state: str) -> None:
        raise NotImplementedError

    def get_backlight_level(self) -> int:
        raise NotImplementedError

    # Haptics
    def get_last_haptic(self) -> Optional[str]:
        raise NotImplementedError

    # Energy view model (optional for now)
    def get_energy_viewmodel(self) -> Dict[str, Any]:
        return {}


class MockBridge(Bridge):
    def __init__(self):
        self.active_app = "Energy"
        self.power_state = "ACTIVE"
        self.backlight = 255
        self.inactivity_timeout = 120
        self.last_haptic: Optional[str] = None
        self.energy_mode = "kWh Today"

    def close(self):
        pass

    def get_active_app(self) -> str:
        return self.active_app

    def set_active_app(self, name: str) -> None:
        self.active_app = name

    def tap(self) -> None:
        if self.power_state == "IDLE":
            self.power_state = "ACTIVE"
            self.last_haptic = None  # policy: no haptic on wake
            self.backlight = 255
        else:
            # Switch app
            order = ["Energy", "Clock", "Home", "Settings", "Weather"]
            idx = order.index(self.active_app)
            self.active_app = order[(idx + 1) % len(order)]
            self.last_haptic = "short"

    def rotate(self, direction: str) -> None:
        if self.power_state == "IDLE":
            # wake only; do not change mode on first rotate
            self.power_state = "ACTIVE"
            self.backlight = 255
            return
        # Only Energy has modes in this mock
        if self.active_app == "Energy" and direction.upper() in ("LEFT", "RIGHT"):
            self.energy_mode = "£ Today" if self.energy_mode == "kWh Today" else "kWh Today"

    def set_inactivity_timeout(self, seconds: int) -> None:
        self.inactivity_timeout = seconds

    def get_power_state(self) -> str:
        return self.power_state

    def set_power_state(self, state: str) -> None:
        self.power_state = state
        self.backlight = 0 if state == "IDLE" else 255

    def get_backlight_level(self) -> int:
        return self.backlight

    def get_last_haptic(self) -> Optional[str]:
        val = self.last_haptic
        self.last_haptic = None  # read-once
        return val

    def get_energy_viewmodel(self):
        return {"mode": self.energy_mode}


class HttpBridge(Bridge):
    def __init__(self, base_url: str):
        if not requests:
            raise RuntimeError("requests not available")
        self.base_url = base_url.rstrip('/')

    def close(self):
        pass

    def _post(self, path: str, data: dict | None = None):
        r = requests.post(self.base_url + path, json=data or {})
        r.raise_for_status()
        return r.json() if r.headers.get('content-type','').startswith('application/json') else r.text

    def _get(self, path: str):
        r = requests.get(self.base_url + path)
        r.raise_for_status()
        return r.json() if r.headers.get('content-type','').startswith('application/json') else r.text

    def get_active_app(self) -> str:
        return self._get('/test/app/active')

    def set_active_app(self, name: str) -> None:
        self._post('/test/app/active', {"name": name})

    def tap(self) -> None:
        self._post('/test/input/tap')

    def rotate(self, direction: str) -> None:
        self._post('/test/input/rotate', {"dir": direction})

    def set_inactivity_timeout(self, seconds: int) -> None:
        self._post('/test/power/timeout', {"seconds": seconds})

    def get_power_state(self) -> str:
        return self._get('/test/power/state')

    def set_power_state(self, state: str) -> None:
        self._post('/test/power/state', {"state": state})

    def get_backlight_level(self) -> int:
        return int(self._get('/test/power/backlight'))

    def get_last_haptic(self):
        return self._get('/test/haptics/last') or None

    def get_energy_viewmodel(self):
        return self._get('/test/energy/viewmodel')


class SerialBridge(Bridge):
    def __init__(self, port: str, baud: int = 115200):
        if not serial:
            raise RuntimeError("pyserial not available")
        self._ser = serial.Serial(port, baud, timeout=2)

    def close(self):
        try:
            self._ser.close()
        except Exception:
            pass

    def _req(self, cmd: str) -> str:
        self._ser.write((cmd + "\n").encode())
        line = self._ser.readline().decode(errors='ignore').strip()
        return line

    # The following commands assume a simple line protocol implemented by the device in CONFIG_TEST_MODE
    def get_active_app(self) -> str:
        return self._req('GET APP')

    def set_active_app(self, name: str) -> None:
        self._req(f'SET APP {name}')

    def tap(self) -> None:
        self._req('INPUT TAP')

    def rotate(self, direction: str) -> None:
        self._req(f'INPUT ROTATE {direction}')

    def set_inactivity_timeout(self, seconds: int) -> None:
        self._req(f'SET TIMEOUT {seconds}')

    def get_power_state(self) -> str:
        return self._req('GET POWER')

    def set_power_state(self, state: str) -> None:
        self._req(f'SET POWER {state}')

    def get_backlight_level(self) -> int:
        return int(self._req('GET BKL'))

    def get_last_haptic(self):
        v = self._req('GET HAPTIC')
        return v if v else None

    def get_energy_viewmodel(self):
        raw = self._req('GET ENERGY VM')
        try:
            return json.loads(raw)
        except Exception:
            return {}


def get_bridge(mode: str = "mock", url: Optional[str] = None, serial_port: Optional[str] = None, serial_baud: int = 115200) -> Bridge:
    mode = (mode or "mock").lower()
    if mode == "mock":
        return MockBridge()
    if mode == "http":
        if not url:
            raise RuntimeError("BRIDGE_URL is required for http mode")
        return HttpBridge(url)
    if mode == "serial":
        if not serial_port:
            raise RuntimeError("SERIAL_PORT is required for serial mode")
        return SerialBridge(serial_port, serial_baud)
    raise RuntimeError(f"Unknown bridge mode: {mode}")
