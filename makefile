NAME        = MINECALC
ICON        = icon.png
DESCRIPTION = "Fully 3D Minecraft like game for the TI-84 + CE! And it even runs at around 7 FPS!!!"
COMPRESSED  = YES
ARCHIVED    = YES
OUTPUT_MAP = YES
# --- CRITICAL FOR SPEED ---
# -ffast-math: Speeds up float math (ignores tiny IEEE errors)
# -fomit-frame-pointer: Frees up a register (IX) for logic instead of stack tracking
CFLAGS   = -Wall -Wextra -O3 -ffast-math -fomit-frame-pointer -fno-exceptions -fno-rtti -fno-threadsafe-statics 
CXXFLAGS = -Wall -Wextra -O3 -ffast-math -fomit-frame-pointer -fno-exceptions -fno-rtti -fno-threadsafe-statics

# Ensure Link Time Optimization is ON (standard in newer CEdev)
LTO = YES

# Use the toolchain's faster C libraries instead of the calculator's slow OS calls
PREFER_OS_LIBC = NO
COMPRESSED_MODE = zx0 #for release only
# ----------------------------

include $(shell cedev-config --makefile)
