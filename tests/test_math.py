import unittest
import pybind11_example


class MainTest(unittest.TestCase):
    def test_add(self):
        self.assertEqual(pybind11_example.add(1, 1), 2)

    def test_subtract(self):
        self.assertEqual(pybind11_example.subtract(1, 1), 0)


if __name__ == '__main__':
    unittest.main()
