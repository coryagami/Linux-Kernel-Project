"""
    To run in user space, simply import logic, then call start_shuttle, then add
    passengers to queue with issue_request, look at shuttle status with logic.shuttle,
    and finish with stop_shuttle.
"""

#! /usr/bin/python3

import logic

if __name__ == "__main__":
    logic.start_shuttle()
    while(logic.shuttle["online"] == True):
        logic.run()
    logic.stop_shuttle()
