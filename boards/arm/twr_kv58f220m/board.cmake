# SPDX-License-Identifier: Apache-2.0

set_ifndef(OPENSDA_FW jlink)

if(OPENSDA_FW STREQUAL jlink)
  board_set_debugger_ifnset(jlink)
  board_set_flasher_ifnset(jlink)
endif()

board_runner_args(jlink "--device=MKV58F1M0xxx24")

include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
