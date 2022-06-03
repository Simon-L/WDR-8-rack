# Dummy Plugin!

Based on the helper.py script from Rack SDK and the Fundamental panels for Cardinal.

```bash
git clone https://github.com/Simon-L/dummy-plugin yourdummyplugin
cd yourdummyplugin
export RACK_SDK=/path/to/sdk
make
ln -s $(pwd) ~/.Rack2/plugins/yourdummyplugin
```

Rename:
```bash
NAME=NewModuleName
sed -i "s/DummyModule/${NAME}/g" src/DummyModule.cpp src/plugin.* plugin.json
mv src/DummyModule.cpp src/${NAME}.cpp
# New panel file?
sed -i "s/dummy-panel/panel-file/g" src/${NAME}.cpp
mv res/dummy-panel.svg res/panel-file.svg
```

Change slug, name and brand manually. Don't forget to update the license ;)

Later on, reuse the old file from initial commit to start a new module:  
`git show 520e96ec1f53f751b05a6ff98e91b205c5ed13d6:src/DummyModule.cpp > src/DummyModule.cpp`  
and use the sed commands above for the cpp file. Oh no, plugin.cpp and plugin.hpp have to be adapted manually, oh no!! it's not automated!

![](https://nextcould.roselove.pink/s/wsG9LS5f33YqcBd/preview)

## License

Panel is copyright © 2022 Jason Corder and Filipe Coelho