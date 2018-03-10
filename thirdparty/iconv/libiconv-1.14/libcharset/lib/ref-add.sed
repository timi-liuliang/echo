/^# Packages using this file: / {
  s/# Packages using this file://
  ta
  :a
  s/  /  /
  tb
  s/ $/  /
  :b
  s/^/# Packages using this file:/
}
