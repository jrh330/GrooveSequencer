#!/bin/bash

# Read the current version
VERSION=$(cat version.txt)

# Split into major, minor, and build numbers
IFS='.' read -r MAJOR MINOR BUILD <<< "$VERSION"

# Increment the build number
NEW_BUILD=$(printf "%02d" $((10#${BUILD} + 1)))

# Create new version string
NEW_VERSION="${MAJOR}.${MINOR}.${NEW_BUILD}"

# Update version.txt
echo $NEW_VERSION > version.txt

# Update both VERSION instances in CMakeLists.txt
sed -i '' "s/VERSION ${VERSION}/VERSION ${NEW_VERSION}/g" CMakeLists.txt

# Clean the build directory to force a full rebuild
rm -rf build/*

echo "Version updated from ${VERSION} to ${NEW_VERSION}"
echo "Build directory cleaned - please run 'cmake -B build && cmake --build build'" 