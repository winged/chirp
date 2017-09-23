"""Test chirp buffers/handlers."""
import mpipe
import buffer_funcs as fnc

from hypothesis.strategies import tuples, sampled_from, just, integers
from hypothesis.stateful import GenericStateMachine


class GenBuffer(GenericStateMachine):
    """Test if the stays consistent."""

    def __init__(self):
        self.reinit()
        self.acquired = set()
        self.size = 0

        self.init_step = tuples(
            just("init"),
            integers(
                min_value=1,
                max_value=32
            )
        )
        self.acquire_step = tuples(just("acquire"), just(0))

    def reinit(self):
        self.proc = mpipe.open(["src/buffer_etest"])
        self.initialized = False

    def teardown(self):
        mpipe.write(self.proc, (fnc.func_cleanup_e, 0))
        assert mpipe.read(self.proc) == [0]
        mpipe.close(self.proc)
        del self.proc  # Hypothesis seems to keep GSM objects

    def steps(self):
        if not self.initialized:
            self.initialized = True
            return self.init_step
        else:
            if self.acquired:
                release_step = tuples(just(
                    "release"
                ), sampled_from(sorted(self.acquired)))
            else:
                release_step = tuples(just(
                    "release"
                ), integers(min_value=0, max_value=self.size - 1))
            return release_step | self.acquire_step

    def execute_step(self, step):
        action, value = step
        if action == 'init':
            self.size = value
            mpipe.write(
                self.proc,
                (fnc.func_init_e, self.size)
            )
            assert mpipe.read(self.proc) == [0]
        elif action == 'acquire':
            mpipe.write(
                self.proc,
                (fnc.func_acquire_e, 0)
            )
            last, id_ = mpipe.read(self.proc)
            if len(self.acquired) == self.size - 1:
                assert last == 1
            elif len(self.acquired) > self.size - 1:
                assert id_ == -1
            else:
                assert last == 0
            if id_ != -1:
                self.acquired.add(id_)
        elif action == 'release':
            if value in self.acquired:
                mpipe.write(
                    self.proc,
                    (fnc.func_release_e, value)
                )
                assert mpipe.read(self.proc) == [0]
                self.acquired.remove(value)
            else:
                try:
                    mpipe.write(
                        self.proc,
                        (fnc.func_release_e, value)
                    )
                    assert mpipe.read(self.proc) == [0]
                except Exception:
                    self.reinit()
                else:
                    assert False, "Test binary has to assert"
        else:
            assert False, "Unknown step"


TestBuffer = GenBuffer.TestCase