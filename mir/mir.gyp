# Copyright 2013 The Chromium Authors. All rights reserved.
# Copyright 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'conditions': [
      ['sysroot!=""', {
        'pkg-config': './pkg-config-wrapper "<(sysroot)" "<(target_arch)"',
      }, {
        'pkg-config': 'pkg-config'
      }],
    ],
  },

  'targets': [
    {
      'target_name': 'mir_toolkit',
      'type': 'static_library',
      'variables': {
        'mir_packages': [
          'mirclient',
          'xkbcommon',
        ],
      },
      'cflags': [
        '<!@(<(pkg-config) --cflags <(mir_packages))',
        '-std=c++11'
      ],
      'direct_dependent_settings': {
        'cflags': [
          '<!@(<(pkg-config) --cflags <(mir_packages))',
        ],
      },
      'link_settings': {
        'ldflags': [
          '<!@(<(pkg-config) --libs-only-L --libs-only-other <(mir_packages))',
        ],
        'libraries': [
          '<!@(<(pkg-config) --libs-only-l <(mir_packages))',
        ],
      },
      'dependencies': [
        '../../base/base.gyp:base',
      ],
      'include_dirs': [
        '../..',
      ],
      'includes': [
        '../ui/ime/ime.gypi',
      ],
      'sources': [
         'mir_window.cc',
         'mir_window.h',
         'mir_display.cc',
         'mir_display.h',
         'mir_pointer.cc',
         'mir_pointer.h',
         'mir_keyboard.cc',
         'mir_keyboard.h'
      ],
    },
  ]
}
