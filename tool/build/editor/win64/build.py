import os
import subprocess
import argparse
import shutil
import time

# root directory
root_dir = os.getcwd()

def release_echo():
	 # dirs
    src_dir = root_dir + '/../../../../'
    des_dir = root_dir + '/nsis/echo/'
    nsis_dir= root_dir + '/nsis/'

    # remove des dir
    shutil.rmtree( des_dir, True)

    # define copy list
    copy_dir_list = [
        "app/",
        "bin/editor/Win64/Release/",
        "editor/echo",
        "tool/build/windows/",
        "tool/build/ios/",
        "tool/build/android/",
        "tool/build/mac/",
        "engine/",
        "thirdparty/",
        "CMakeLists.txt",
    ]

    # copy files
    for sub_dir in copy_dir_list:
        full_dir = src_dir + sub_dir
        if os.path.isdir(full_dir):
            shutil.copytree( full_dir, des_dir + sub_dir)
            print('copy resource from [' + src_dir + sub_dir + '] to [' + des_dir + sub_dir + ']')
        else:
            shutil.copyfile( full_dir, des_dir + sub_dir)
            print('copy resource from [' + src_dir + sub_dir + '] to [' + des_dir + sub_dir + ']')

	# generate installer
    astudio_version_name = 'echo-setup-' + time.strftime('%Y.%m.%d',time.localtime(time.time())) + '.exe'
    os.chdir( nsis_dir)
    subprocess.call('makensis.exe echo.nsi')
    os.rename('echo-setup.exe', astudio_version_name)
    #shutil.move(astudio_version_name, astudio_des_dir+astudio_version_name)

    return

# function parse args
def run_parse_args() :
    # create an ArgumentParser object
    parser = argparse.ArgumentParser(description='build LordEngine project on Windows...')
    
    parser.add_argument('-nsis', help='nsis build', choices=['echo'])

    # parse instruction
    args = parser.parse_args()

    if args.nsis=='echo':
        release_echo()

    # end of the function parse_args
    return

if __name__ == '__main__' :
    args = run_parse_args()
