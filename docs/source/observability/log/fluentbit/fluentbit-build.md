# Build Fluentbit


```bash
sudo apt install cmake
sudo apt install flex bison
sudo apt install libyaml-dev
```

```bash
cd build/
cmake ../ -DFLB_DEBUG=On

make
```