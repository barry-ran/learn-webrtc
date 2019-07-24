import os
import subprocess
import json

# https://github.com/Microsoft/vswhere/blob/4b16c6302889506e2d49ff24cfa39234753412b2/README.md
_VSWHERE_PATH = r'%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe'

def DetectVisualStudioPath(version_as_year):
  """Return path to the version_as_year of Visual Studio.
  """      

  year_to_version = {
    '2013': '12.0',
    '2015': '14.0',
    '2017': '15.0',
  }

  if version_as_year not in year_to_version:
    raise Exception(('Visual Studio version %s (from version_as_year)'
                     ' not supported. Supported versions are: %s') % (
                      version_as_year, ', '.join(year_to_version.keys())))

  if version_as_year == '2017':
    # The VC++ 2017 install location needs to be located using COM instead of
    # the registry. For details see:
    # https://blogs.msdn.microsoft.com/heaths/2016/09/15/changes-to-visual-studio-15-setup/
    vswhere_path = os.path.expandvars(_VSWHERE_PATH)
    if os.path.exists(vswhere_path):
        version = year_to_version[version_as_year]
        try:
          out = json.loads(subprocess.check_output([
              vswhere_path, '-version',
              '[{},{})'.format(float(version), float(version) + 1),
              '-legacy', '-format', 'json', '-utf8',
          ]))
          if out:
              return out[0]['installationPath']
        except subprocess.CalledProcessError:
          pass

    for drive_letter in ['C', 'D', 'E', 'F']:
      root_path = r'%s:\Program Files (x86)\Microsoft Visual Studio\2017' % drive_letter
      for edition in ['Professional', 'Community', 'Enterprise', 'BuildTools']:
        path = os.environ.get('VS2017_OVERRIDE_PATH', os.path.join(root_path, edition))
        if os.path.exists(path):
          return path
  else:
    version = year_to_version[version_as_year]
    keys = [r'HKLM\Software\Microsoft\VisualStudio\%s' % version,
            r'HKLM\Software\Wow6432Node\Microsoft\VisualStudio\%s' % version]
    for key in keys:
      path = _RegistryGetValue(key, 'InstallDir')
      if not path:
        continue
      path = os.path.normpath(os.path.join(path, '..', '..'))
      return path

  raise Exception(('Visual Studio Version %s (from version_as_year)'
                   ' not found.') % (version_as_year))

if __name__ == '__main__':    
    try:
        print DetectVisualStudioPath('2017')
        exit(0)
    except Exception, e:
        # print e.message
        exit(1)