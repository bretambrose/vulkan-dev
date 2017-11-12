
import argparse
import os
import pdb
import shutil
import subprocess

def MakeParser():
    parser = argparse.ArgumentParser(description="Shader Compilation Script")
    parser.add_argument("--clean", action="store_true")
    parser.add_argument("--source_directory", action="store")
    parser.add_argument("--dest_directory", action="store")

    return parser

def Validate(sourceDir, destDir):
    if sourceDir is None or not os.path.exists(sourceDir):
        print ('Error: Invalid source directory')
        return False

    if destDir is None:
        print ('Error: Invalid dest directory')
        return False

    return True

def CompileShaders(sourceDir, destDir, compilerPath):
    for rootDir, dirNames, fileNames in os.walk( sourceDir ):
        for fileName in fileNames:
            sourceFilePath = os.path.join(rootDir, fileName)
            sourceFileModifiedTime = os.path.getmtime(sourceFilePath)

            relPath = os.path.relpath(rootDir, sourceDir)
            shaderDestFilename, extension = os.path.splitext(fileName)
            periodIndex = extension.find('.')
            if periodIndex >= 0:
                extension = extension[(periodIndex+1):]
                
            shaderDestFilename = shaderDestFilename + "_" + extension + ".spv"

            shaderDestDirectory = os.path.join(destDir, relPath)
            shaderDestFilePath = os.path.join(shaderDestDirectory, shaderDestFilename)

            destFileModifiedTime = 0
            if os.path.isfile(shaderDestFilePath):
                destFileModifiedTime = os.path.getmtime(shaderDestFilePath)

            if sourceFileModifiedTime >= destFileModifiedTime:
                print ('Compiling ' + sourceFilePath + ' into ' + shaderDestFilePath + '...')

                commandLine = '"' + compilerPath + '"' + ' -V -o "' + shaderDestFilePath + '"  "' + sourceFilePath + '"'

                return_code = subprocess.call(commandLine, shell=True)
                if return_code != 0:
                    print ('Error: ' + return_code)

    return

def Main():

    parser = MakeParser()
    args = vars( parser.parse_args() )

    clean = args[ "clean" ]
    sourceDir = args[ "source_directory" ] or "."
    destDir = args[ "dest_directory" ]
    
    if not Validate(sourceDir, destDir):
        parser.print_help()
        return 1

    sdkPath = os.environ['VULKAN_SDK']
    if sdkPath is None or not os.path.exists(sdkPath):
        print ('Error: Could not find Vulkan SDK')
        return 1

    compilerPath = os.path.join(sdkPath, "Bin32", "glslangValidator.exe")
    if not os.path.isfile(compilerPath):
        print('Error: could not find glslangValidator.exe')
        return 1

    print ( 'Compiling shaders...' )

    if clean and os.path.exists(destDir):
        print( 'Cleaning destination directory ' + destDir + '...' )
        shutil.rmtree(destDir)

    if not os.path.exists(destDir):
        print( 'Destination directory ' + destDir + ' does not exist, creating...' )
        os.makedirs( destDir )

    CompileShaders(sourceDir, destDir, compilerPath)

    print ( 'Finished\n' )
    return 0

Main()
