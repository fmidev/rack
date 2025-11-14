# Python3 utilities for Rack

Rack comes with a set of Python scripts packed as a module [rack](./rack).

## Contents

- [`rack.composer`](rack/composer.py) - utility for extracting metadata from ODIM-HDF5 files, esp. for monitoring incoming data
- [`rack.statistics`](rack/statistics.py) - utility for extracting metadata from ODIM-HDF5 files, esp. for monitoring incoming data


### Statistics

## Examples

In the following examples, `$SCR` equals to the absolute path of `rack/scripts`. Further, `$PYTHONPATH` is assumed to contain it.
```bash
# Daywise file of for sweep (identified by dataset<N>):
python3 $SRC/rack/statistics.py  --OUTDIR './stats1/{SITE}/{MINUTE}min/dataset{DATASET}'  --OUTFILE '{MONTH}{DAY}_{POL}_{ELANGLE}_{PRF}_{GEOM}.txt' data-acc/201703061200_radar.polar.fiuta.h5

# Files for each volume, separated by polarization modes and pulse repetition modes:
python3 $SRC/rack/statistics.py  --OUTDIR './stats1/{SITE}'  --OUTFILE '{TIMESTAMP}_{POL}_{PRF}.txt' 
```
Produces:
```
Test
```


test.py
```python
import rack.statistics

parser = rack.statistics.build_parser()
args = parser.parse_args()

rack.statistics.run(args)
```

```bash
# PYTHONPATH=$PYTHONPATH:<path..>/scripts
python3 ./test.py   volume.h5
```
Produces:
```
[./statistics/fiuta/00min/01][2014-08-27T00:00_00.30_570_500x360x500]:  2014-08-27T00:00 2014-08-27T00:00 00.30 # TH-DBZH-VRAD-WRAD-ZDR-KDP-RHOHV-DBZHC-SQI-PHIDP-HCLASS-ZDRC-TX-DBZX
[./statistics/fiuta/00min/02][2014-08-27T00:00_00.70_570_500x360x500]:  2014-08-27T00:00 2014-08-27T00:24 00.70 # TH-DBZH-VRAD-WRAD-ZDR-KDP-RHOHV-DBZHC-SQI-PHIDP-HCLASS-ZDRC-TX-DBZX
[./statistics/fiuta/00min/03][2014-08-27T00:00_01.50_570_500x360x500]:  2014-08-27T00:00 2014-08-27T00:47 01.50 # TH-DBZH-VRAD-WRAD-ZDR-KDP-RHOHV-DBZHC-SQI-PHIDP-HCLASS-ZDRC-TX-DBZX
[./statistics/fiuta/00min/04][2014-08-27T00:00_03.00_570_500x360x500]:  2014-08-27T00:00 2014-08-27T01:10 03.00 # TH-DBZH-VRAD-WRAD-ZDR-KDP-RHOHV-DBZHC-SQI-PHIDP-HCLASS-ZDRC-TX-DBZX
[./statistics/fiuta/00min/05][2014-08-27T00:00_05.00_570_459x360x500]:  2014-08-27T00:00 2014-08-27T01:34 05.00 # TH-DBZH-VRAD-WRAD-ZDR-KDP-RHOHV-DBZHC-SQI-PHIDP-HCLASS-ZDRC-TX-DBZX
[./statistics/fiuta/00min/06][2014-08-27T00:00_09.00_570_256x360x500]:  2014-08-27T00:00 2014-08-27T01:57 09.00 # TH-DBZH-VRAD-WRAD-ZDR-KDP-RHOHV-DBZHC-SQI-PHIDP-HCLASS-ZDRC-TX-DBZX
[./statistics/fiuta/00min/07][2014-08-27T00:00_02.00_1200:900_248x360x500]:  2014-08-27T00:00 2014-08-27T02:22 02.00 # TH-DBZH-VRAD-WRAD-SQI
[./statistics/fiuta/00min/08][2014-08-27T00:00_07.00_1200:900_248x360x500]:  2014-08-27T00:00 2014-08-27T02:38 07.00 # TH-DBZH-VRAD-WRAD-SQI
[./statistics/fiuta/00min/09][2014-08-27T00:00_11.00_1200:900_210x360x500]:  2014-08-27T00:00 2014-08-27T02:53 11.00 # TH-DBZH-VRAD-WRAD-SQI
[./statistics/fiuta/00min/10][2014-08-27T00:00_15.00_1200:900_155x360x500]:  2014-08-27T00:00 2014-08-27T03:09 15.00 # TH-DBZH-VRAD-WRAD-SQI
[./statistics/fiuta/00min/11][2014-08-27T00:00_25.00_1200:900_95x360x500]:  2014-08-27T00:00 2014-08-27T03:25 25.00 # TH-DBZH-VRAD-WRAD-SQI
[./statistics/fiuta/00min/12][2014-08-27T00:00_45.00_1200:900_57x360x500]:  2014-08-27T00:00 2014-08-27T03:42 45.00 # TH-DBZH-VRAD-WRAD-SQI
[./statistics/fiuta/00min/13][2014-08-27T00:00_00.40_1200:900_992x360x125]:  2014-08-27T00:00 2014-08-27T04:00 00.40 # TH-DBZH-VRAD-WRAD-SQI
```


Explanation

| A                   | B                | C              | D          |
| ------------------- | ---------------- | -------------  | ---------- |
| Default             | `/dir`           | Test           | (None)     |



 
# Local build

