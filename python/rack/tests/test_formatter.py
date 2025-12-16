
import unittest
import datetime as dt
from rack.formatter import smart_format

class TestFormatter(unittest.TestCase):


    def test_smart_format(self):

        data = {
            "name": "Ada",
            "value": -3.1415926,
            "time": dt.datetime(2025, 11, 3, 14, 25),
            "path": "/home/ada/data/results.txt",
            "seq": [1, 2, 3, 4],
            "unneeded": 1,
        }

        test_set = [
            ("Hello {name|upper}!", "Hello ADA!"),
            ("Time: {time|%Y-%m-%d %H:%M}", "Time: 2025-11-03 14:25"),
            ("Rounded: {value|abs|.2f}", "Rounded: 3.14"),
            ("File: {path|basename|upper}", "File: RESULTS.TXT"),
            ("Number of items: {seq|len|.0f}", "Number of items: 4"),
        ]

        for tpl, expected in test_set:
            result = smart_format(tpl, data)
            self.assertEqual(result, expected)  



if __name__ == "__main__":
    unittest.main()
