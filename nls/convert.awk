/^\$set/ {
  major++
  minor = 0

  if (major > 1)
    printf "\n" > output
  printf "$set %d %s\n", major, $3 > output

  if (header) {
    majorName = substr($3, 2)
    if (major > 1)
      printf "\n" > header
    printf "#define %sSet %#x\n", majorName, major > header
  }
}

/^\$ #/ {
  minor++

  if (header) {
    minorName = substr($2, 2)
    printf "#define %s%s %#x\n", majorName, minorName, minor > header
  }
}

/^#/ {
  text = substr($0, 3)  
  printf "%d %s\n", minor, text > output
}
  
! /^(\$|#)/ { print > output }

