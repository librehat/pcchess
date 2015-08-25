#Parallel Chinese Chess

##Dependencies

- Boost >= 1.57 (components: mpi, serialization, date_time, program_options)
- CMake >= 2.8
- PThread
- GCC >= 5.1.1 (or clang >= 3.4)\*
- OpenMP (optional but recommended)
- gperftools (Google Performance Tools, optional and only used in debug mode)

\*: A new version of `libstdc++` (>= 5.1.1) or `libc++` is needed

##Compiling
```
mkdir build && cd build
cmake ..
make -j8
```

It'll build executables `pcc_mpi` and `pcc_serial`, along with libraries `libpcc_core.so` and `libpcc_player.so`.For executables command-line options, please run `./pcc_serial -h` and `./pcc_mpi -h`.

##License
![](http://www.gnu.org/graphics/lgplv3-147x51.png)

This project is licensed under version 3 of the GNU Lesser General Public License. Check LICENSE file for details.
