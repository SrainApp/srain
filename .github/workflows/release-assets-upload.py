#!/usr/bin/python3
# A python script to upload the release assets via the GitHub Release API.
import sys, os, json, requests, mimetypes

# Written for Github actions when release created event is triggered.
# It must set the env variables in the GitHub actions file:
#      env:
#        GH_TOKEN: ${{ secrets.GITHUB_TOKEN }}
#        GH_REPO: ${{ github.repository }}
#        GH_REF: ${{ github.ref }}
GITHUB_TOKEN = os.environ["GH_TOKEN"];
GITHUB_REPOSITORY = os.environ["GH_REPO"];
GITHUB_REF = os.environ["GH_REF"];
GITHUB_TAGNAME = GITHUB_REF.replace(u'refs/tags/','');

# List releases for a repository:
# https://developer.github.com/v3/repos/releases/#list-releases-for-a-repository
RELEASES_API = "https://api.github.com/repos/" + GITHUB_REPOSITORY + "/releases";

# The path of assets, eg. "out/".
OUT_FILES_PATH = "out/";

def get_uploadurl():
  global UPLOAD_URL;

  print("Releases API: " + RELEASES_API);

  headers = {
    'Authorization': 'token ' + GITHUB_TOKEN,
  };

  response = requests.get(RELEASES_API, headers=headers);

  print("debug: " + str(response.status_code));

  RELEASES = response.json();
  RELEASES_LEN = len(RELEASES);

  for i in range (0, RELEASES_LEN):
    if RELEASES[i]["tag_name"] == GITHUB_TAGNAME:
      RELEASES_NUMBER = i;
      break;

  try:
    print("Found the target tagname, " + str(RELEASES_NUMBER));

  except:
    print("Can't found the target tagname.");
    sys.exit(1);

  UPLOAD_URL = RELEASES[RELEASES_NUMBER]["upload_url"].replace(u'{?name,label}','');

  print("Upload URL: " + UPLOAD_URL);

def upload_assets():
  OUT_FILES = os.listdir(OUT_FILES_PATH);
  OUT_FILES_NUMBER = len(OUT_FILES);
  print("Assets: " + str(OUT_FILES) + "\nAssets number: " + str(OUT_FILES_NUMBER));

  FAILURE_MARK = "0";

  for i in range (0, OUT_FILES_NUMBER):
    FILENAME = OUT_FILES[i];
    print("Current asset: " + FILENAME);

    # Upload a release asset:
    # https://developer.github.com/v3/repos/releases/#upload-a-release-asset

    # `Content-Type` is required, use `mimtypes` to guess the file's mimetype.
    MIMETYPE = mimetypes.guess_type(OUT_FILES_PATH + FILENAME)[0];

    # Use `application/octet-stream` for an unknown file type.
    if MIMETYPE is None:
      MIMETYPE = "application/octet-stream";

    print("Mimetype: " + MIMETYPE);

    headers = {
      'Authorization': 'token ' + GITHUB_TOKEN,
      'Content-Type': MIMETYPE,
    };

    params = (
      ('name', FILENAME),
    );

    data = open(OUT_FILES_PATH + FILENAME, 'rb').read();

    response = requests.post(UPLOAD_URL, headers=headers, params=params, data=data);

    print("debug: " + str(response.status_code) + "\ndebug:\n" + str(response.text));

    # Response for successful upload: 201 Created
    # https://developer.github.com/v3/repos/releases/#response-for-successful-upload
    if response.status_code == 201:
      print("\033[1;32;40m" + FILENAME + ": success. " + str(i+1) + "/" + str(OUT_FILES_NUMBER) + "\033[0m");

    else:
      print("\033[1;31;40m" + FILENAME + ": fail. " + str(i+1) + "/" + str(OUT_FILES_NUMBER) +  "\033[0m");
      FAILURE_MARK = "1";

  if FAILURE_MARK == "1":
    sys.exit(1);

  elif FAILURE_MARK == "0":
    sys.exit(0);

get_uploadurl();
upload_assets();
