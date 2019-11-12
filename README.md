# vcvrack-testplugin

Test plugin to experiment with VCV Rack modules.

It contains the tutorial plugin from the online manual: https://vcvrack.com/manual/PluginDevelopmentTutorial.html

## Documentation

The VCV Rack API v1 is here: https://vcvrack.com/docs/index.html 

- How to create a context menu item
  - https://community.vcvrack.com/t/adding-context-menu-modes-to-your-module/5865/2

## Debugging

See /rack-sdk/include/logger.hpp for levels of log messages, for example

```c++
DEBUG("Hello world");
```

Log messages are stored in log.txt.<br>
To follow the changes in log.txt in the terminal:

```bash
tail -f /Users/<username>/Documents/Rack/log.txt 
```
