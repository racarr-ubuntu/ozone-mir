# Copyright 2013 The Chromium Authors. All rights reserved.
# Copyright 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'mir_toolkit',
      'type': 'static_library',
      'variables': {
        'MIR_VERSION': '1.2.0',
        'MESA_VERSION': '9.1.3',
        'mir_packages': [
          'egl >= <(MESA_VERSION)',
          'mirclient >= <(MIR_VERSION)',
          'xkbcommon',
        ],
      },
      'cflags': [
        '<!@(<(pkg-config) --cflags <(mir_packages))',
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
        '<(DEPTH)/third_party/khronos',
      ],
      'sources': [
        'mir_display.cc',
        'mir_display.h'
      ],
    },
  ]
}
