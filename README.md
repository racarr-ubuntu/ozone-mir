# Introduction

Ozone-Mir is an implementation of Chromiums Ozone layer for the Mir Windowing system. Ozone Mir allows the Chromium browser and other projects based off of Chromium to run within a Mir session.

You can read a little about the history and design of Ozone Mir here: https://plus.google.com/104358235952874475094/posts/cPmZofCRSA3

# Building Ozone Mir

Instructions are given from Ubuntu 14.04. First you will need all the build dependencies:

  ```
  $ sudo apt-get build-dep chromium-browser
  $ sudo apt-get build-dep libmirclient-dev
  ```

In order to clone the Chromium tree we need the gclient tools:

   ``` 
   $ mkdir src/chromium
   $ cd src/chromium
   $ git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
   $ export PATH=`pwd`/depot_tools:$PATH
   ```

Now we configure gclient to point at ozone Mir and set appropriate options (this may take a long time!):

  ```
  $ gclient config <OZONEMIRURLHERE>
  $export GYP_DEFINES='use_ash=0 use_aura=1 chromeos=0 use_ozone=1' gclient sync
  ```

If you expect to make frequent use consider exporting GYP_DEFINES and PATH in your bashrc.

Its necessary to apply some patches to the chromium tree:

  ```
  $ src/ozone/patches/patch-chromium.sh
  ```

At this point just select a target and build with Ninja!

  ```
  $ ninja -C out/Release -j4 chrome # Release build of full browser
  $ ninja -C out/Debug -j4 content_shell # Debug build of simple browser using Blink content API
  ```

You should be able to run the produced binary against a running Mir server:

  ```
  $ ./out/Release/chrome --no-sandbox
  ```

# Contact Information

mir-devel@lists.ubuntu.com or #ubuntu-mir on FreeNode!
