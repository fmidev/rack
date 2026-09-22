import unittest
import logging

import rack.maps
# import rack

# logging.basicConfig(format='%(levelname)s\t %(name)s: %(message)s')
logging.basicConfig(format='%(levelname)s:\t %(message)s')
logger = logging.getLogger() 
logger.setLevel(logging.INFO)


class TestMaps(unittest.TestCase):
    """
        Note: command line arguments of 'unittest' confuse or corrupt the initialisation
        of rack.prog.RackModule 
    """

    def test_map_layers(self):

        root = rack.maps.get_wms_capabilities("https://ows.mundialis.de/osm/service")

        # rack.maps.show_wms_layers(root)

    def test_map_layer(self):
        root = rack.maps.get_wms_capabilities("https://ows.mundialis.de/osm/service")

        rack.maps.show_wms_layer(root, "TOPO-WMS")

    def test_crs(self):
        self.assertEqual(rack.maps.bbox_needs_swap("1.1.0", 4326), False)
        self.assertEqual(rack.maps.bbox_needs_swap("1.3.0", 3035), True)
                
        self.assertEqual(rack.maps.bbox_needs_swap("1.3.0", "EPSG:4326"), True)
        self.assertEqual(rack.maps.bbox_needs_swap("1.3.0", "EPSG:3035"), True)
        self.assertEqual(rack.maps.bbox_needs_swap("1.3.0", "EPSG:3067"), False)
        self.assertEqual(rack.maps.bbox_needs_swap("1.1.1", "EPSG:3035"), False)

