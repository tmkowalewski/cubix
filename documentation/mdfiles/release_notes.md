/** @page release-notes Release Notes

## Version 1.0

Initial release : Wed, 6 Dec 2022

## Version 1.1

### tknucleus

* Adding the methods [is_z_magic()](@ref tkn::tknucleus::is_z_magic()), [is_n_magic()](@ref tkn::tknucleus::is_n_magic()) and [is_doubly_magic()](@ref tkn::tknucleus::is_doubly_magic())

### tklevel

* Adding the information on yrast levels, and [is_yrast()](@ref tkn::tklevel::is_yrast()) method

### examples

* Adding the deformation example
* Debug in the tkn-thread-example (removing the rand() generator that is not thread-safe, replaced by modern c++ random generator)
