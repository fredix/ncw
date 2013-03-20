# NCW

ncw is the nodecast worker. Go to http://nodecast.github.com/ncw/

## INSTALL

install some debs :

```bash
apt-get install g++ make autoconf scons libboost-all-dev libqt4-dev libavahi-compat-libdnssd-dev
```

get submodule libraries :

```bash
git submodule update --init
```

make the zeromq lib :

```bash
cd externals/zeromq
./configure
make
sudo make install
```
install libqxt or your distribution's package libqxt-dev (>= 0.6.2)

```bash
cd external/libqxt
./configure
make
sudo make install
sudo echo "/usr/local/Qxt/lib/" >> /etc/ld.so.conf.d/x86_64-linux-gnu.conf
sudo ldconfig
```

make the mongodb client :

```bash
cd externals/mongo-cxx-driver/
scons mongoclient
```

last, make the nodecast worker :

```bash
qmake
make
mkdir ~/bin
cp ncw ~/bin
```


## WORKER

Launch a worker :

```bash
ncw --ncs-ip=127.0.0.1 --ncs-port=5569 --node-uuid=your-node-uuid --node-password=your-node-password --worker-type=process --worker-name="ls" --exec="ls" --stdout=true --directory=$HOME/ncw_data/
```

Launch a service :

```bash
ncw --ncs-ip=127.0.0.1 --ncs-port=5569 --node-uuid=your-node-uuid --node-password=your-node-password --worker-type=service --worker-name="bttrack" --exec="/usr/bin/bttrack --bind 0.0.0.0 --port 6969 --dfile dstate" --stdout=true --directory=$HOME/ncw_data/
```

contact : fredix at nodecast dot net
