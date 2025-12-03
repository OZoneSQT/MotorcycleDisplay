from typing import Iterable, Optional

class Message:
    arbitration_id: int
    data: bytes
    is_extended_id: bool

    def __init__(self, *, arbitration_id: int, data: Iterable[int], is_extended_id: bool) -> None: ...

class BusABC:
    def send(self, message: Message, timeout: Optional[float] = ...) -> None: ...

class Bus(BusABC):
    ...
