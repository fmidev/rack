import unittest
from unittest.mock import patch

from rack.doxymoron import Block, CliConf, run_shell


class TestRunShell(unittest.TestCase):

    def test_backslash_continuations_join_physical_lines(self):
        block = Block(
            kind="sh",
            arg=None,
            content=["rack --first \\", "  --second \\", "  --third", "rack --next"],
            start_line=1,
        )
        commands = []

        with patch(
            "rack.doxymoron.rack.process.run",
            side_effect=lambda command, **kwargs: commands.append(command),
        ):
            run_shell(block, CliConf())

        self.assertEqual(
            commands,
            ["rack --first --second --third", "rack --next"],
        )


if __name__ == "__main__":
    unittest.main()
