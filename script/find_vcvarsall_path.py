import os
import subprocess
import json


def DetectVCVarSallPath():  
  for drive_letter in ['C', 'D', 'E', 'F']:
      root_path = r'%s:\Program Files (x86)\Microsoft Visual Studio\2017' % drive_letter
      print root_path
      for edition in ['Professional', 'Community', 'Enterprise', 'BuildTools']:
          path = os.path.join(root_path, edition)
          print path
          path = os.path.join(path, r'VC\Auxiliary\Build\vcvarsall.bat')
          print path
          if os.path.exists(path):
              return path
  
  raise Exception('vcvarsall not found.')

if __name__ == '__main__':    
    try:
        print DetectVCVarSallPath()
        exit(0)
    except Exception, e:
        # print e.message
        exit(1)