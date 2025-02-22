if ! cd "cmake-build-debug"; then
  echo "build folder must exist before refresh can run";
  exit 1;
fi

echo -n "Working in dir: "
echo $(pwd);

if ! [ -d "${PY3D_TEST_PROJECT_LOCATION}" ]; then
  echo "env var \"PY3D_TEST_PROJECT_LOCATION\" must be set to an absolute path to a directory before refresh can run";
  exit 1;
fi

set -- "Components" "Materials" "Meshes" "Scenes" "Shaders" "Sprites" "Textures" "config.json"
for item in "$@"; do
  echo "Deleting ${item}";
  rm -rf ${item}
done

for item in "$@"; do
  loc="${PY3D_TEST_PROJECT_LOCATION}/${item}"
  echo "Copying ${loc}"
  if ! cp -r ${loc} .; then
    echo "Project refresh failed, please investigate and then retry";
    exit 1;
  fi
done

exit 0