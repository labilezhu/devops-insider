# Build Fluentbit


```bash
sudo apt install cmake
sudo apt install flex bison
```

```bash
cd build/
cmake ../ -DFLB_CONFIG_YAML=Off -DFLB_DEBUG=On

make
```