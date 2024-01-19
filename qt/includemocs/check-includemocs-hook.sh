# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
#! /bin/sh

# This is an example of a git pre-commit hook.
# Replace the script variable below with the correct location of the script in your setup,
# plus of course the excludes that make sense in your setup.

script="3rdparty/KDToolBox/qt/includemocs/includemocs.py"
if [ ! -e "$script" ]; then
  echo "Failed to find script $script"
  exit 1
fi

python3 "$script" --dry-run --exclude 3rdparty --quiet

if [ $? -ne 0 ]; then
  python3 "$script" --dry-run --exclude 3rdparty
  echo "Run the following command to fix this:"
  echo "python3 $script --exclude 3rdparty"
  exit 1
fi

exit 0
