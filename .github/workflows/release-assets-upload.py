#!/usr/bin/python3
# A python script to upload the release assets via the GitHub Release API.
import json
import mimetypes
import os
import requests
import sys

# Written for Github actions when release created event is triggered.
# It must set the env variables in the GitHub actions file:
#      env:
#        GH_TOKEN: ${{ secrets.GITHUB_TOKEN }}
#        GH_REPO: ${{ github.repository }}
#        GH_REF: ${{ github.ref }}
GithubToken = os.environ["GH_TOKEN"]
GithubRepository = os.environ["GH_REPO"]
GithubRef = os.environ["GH_REF"]
GithubTagname = GithubRef.replace(u'refs/tags/','')

# List releases for a repository:
# https://developer.github.com/v3/repos/releases/#list-releases-for-a-repository
releasesAPI = "https://api.github.com/repos/" + GithubRepository + "/releases"

# The hardcoded path of assets, eg. "out/".
outputFilePath = "out/"

def get_uploadurl():
  global uploadURL

  print("Releases API: " + releasesAPI)

  headers = {
    'Authorization': 'token ' + GithubToken,
  }

  response = requests.get(releasesAPI, headers=headers)

  print("API status code: " + str(response.status_code))

  releases = response.json()
  releasesLength = len(releases)

  for i in range (0, releasesLength):
    if releases[i]["tag_name"] == GithubTagname:
      releasesNumber = i
      break

  try:
    print("Found the target tagname, continue... " + str(releasesNumber))

  # In case the tag was deleted before this python script running.
  except Exception:
    print("Can't found the target tagname, exit.")
    sys.exit(1)

  uploadURL = releases[releasesNumber]["upload_url"].split(u"{")[0]

  print("Upload URL: " + uploadURL)

def upload_assets():
  outputFiles = os.listdir(outputFilePath)
  outputFilesNumber = len(outputFiles)
  print("Assets: " + str(outputFiles) + "\nAssets number: " + str(outputFilesNumber))

  ifFailure = False;

  for i in range (0, outputFilesNumber):
    filename = outputFiles[i]
    print("Current asset: " + filename)

    # Upload a release asset:
    # https://developer.github.com/v3/repos/releases/#upload-a-release-asset

    # `Content-Type` is required, use `mimetypes` to guess the file's mimetype.
    mimetype = mimetypes.guess_type(outputFilePath + filename)[0]

    # Use `application/octet-stream` for an unknown filetype.
    if mimetype is None:
      mimetype = "application/octet-stream"

    print("Mimetype: " + mimetype)

    headers = {
      'Authorization': 'token ' + GithubToken,
      'Content-Type': mimetype,
    }

    params = (('name', filename),)

    data = open(outputFilePath + filename, 'rb').read();

    response = requests.post(uploadURL, headers=headers, params=params, data=data)

    # For debugging.
    print("API status code: " + str(response.status_code))
    #print("API response:\n" + str(response.text))

    # Response for successful upload: 201 Created
    # https://developer.github.com/v3/repos/releases/#response-for-successful-upload
    if response.status_code == 201:
      print("Asset download url: " + response.json()["browser_download_url"])
      print(
        "\033[1;32;40m["
        + str(i + 1)
        + "/"
        + str(outputFilesNumber)
        + "]"
        + filename
        + ": success. "
        + "\033[0m"
      )

    else:
      print(
        "\033[1;31;40m["
        + str(i + 1)
        + "/"
        + str(outputFilesNumber)
        + "]"
        + filename
        + ": fail. "
        + "\033[0m"
      )
      ifFailure = True

  if ifFailure == True:
    sys.exit(1)

  elif ifFailure == False:
    sys.exit(0)

if __name__ == '__main__':
  get_uploadurl()
  upload_assets()