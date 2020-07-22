if [ -z $VULKAN_SDK_PATH ]; then
  if [ ! -f .env_vars ]; then
    printf "Vulkan path: "
    read vulkansdkpath
    echo -e "#!/bin/sh\nexport VULKAN_SDK_PATH='$vulkansdkpath'" > .env_vars
  fi
  source ./.env_vars
  echo "All set up"
fi