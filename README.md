# acmacs-map-draw
Drawing antigenic maps

# Apache module mod_acmacs:

Add to httpd.conf
```
LoadModule acmacs_module "mod_acmacs.so"
AddHandler acmacs .ace .save .save.xz .acd1 .acd1.xz
```

# TODO

- select by lineage
- comparing_with_previous: select antigens not found in another chart

- antigenic time series

- match antigens and sera of two charts
- procrustes
- show_connection_lines
- show_error_lines
- blobs

- generate and use settings in json
- writing plot spec and transformation to .ace
- writing lispmds save
