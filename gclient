solutions = [
  {
    "name": "src",
    "url": "https://webrtc.googlesource.com/src.git",
    "managed": False,
    "custom_deps": {
        #"src/testing": None,
        "src/third_party/gtest-parallel": None,
        "src/third_party/accessibility_test_framework": None,
        "src/third_party/android_support_test_runner": None,
        "src/tools/swarming_client": None,
        "src/tools/luci-go": None,
    },
    "custom_hooks": [
        {"name": "test_fonts"},
        {"name": "Generate component metadata for tests"},
        # The download hook... hopefully
        {"name": ""},
    ],
  },
]
