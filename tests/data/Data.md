

Retrieving data
===============


For more information, descend to directory `rack/tests/data` and run `make help`.




For developers
==============

Adding data to examples and tests
---------------------------------


# Download data manually
  # For example, FMI data:
  ```
  scripts/rack-get-data-fmi.sh 2026{08..10}{05,10,15,20,25}1200_radar.polar.fikau.h5
  ```
  or
  ```
  scripts/rack-get-data-fmi.sh 202609261800_radar.polar.fi{anj,kan,kau,luo,nur,kes,kor,kuo,pet,uta,vih,vim}.h5
  ```

# Link the file(s) to the example directories
  # Example:
  ```	
  env/$ENV
  mkdir data
  cd data/
  ln -s ../../../data-fmi
  ln -s data-fmi/2026-09-26/fi???/202609261800_radar.polar.fi???.h5  .
  ```

