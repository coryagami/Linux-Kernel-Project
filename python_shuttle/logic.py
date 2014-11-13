#! /usr/bin/python3

import time

list = []
shuttle = { "status":"OFFLINE" , "capacity":50, "load":0, "direction":"forward" ,
        "pass_now":{"C":0,"A":0,"L":0}, "curr_term":3, "dest_term":0 , "passengers":[], 
        "pass_had":{"C":0,"A":0,"L":0} , "online":False}
weight = { "C":1, "A":2, "L":4 }
tcoeff = .1

def start_shuttle():
    if(shuttle["online"] == False):
        shuttle["online"] = True
        shuttle["status"] = "PARKED"
        moveto(3)
        return 0
    else:
        print("shuttle already started.\n")
        return 1

def stop_shuttle():
    if(shuttle["status"] != "DEACTIVATING" and shuttle["status"] != "OFFLINE"):
        shuttle["status"] = "DEACTIVATING"
        while(shuttle["load"] > 0):
            run()
        moveto(3)
        shuttle["online"] = "FALSE"
        shuttle["status"] = "PARKED"
        return 0
    else:
        print("shuttle already deactivating.\n")
        return 1

def issue_request(pass_type, init_term, dest_term):
    if((pass_type != "C" and pass_type != "A" and pass_type != "L") or
        init_term > 5 or init_term < 1 or dest_term > 5 or dest_term < 1):
        print("invalid request for passenger(", pass_type, init_term, dest_term, ")\n")    
        return 1
    else:
        p = (pass_type, init_term, dest_term)
        list.append(p)
        return 0

def run():
    moveto(shuttle["dest_term"])

    people_getting_onoff = 0

    # removing people
    tmp_passengers = [person for person in shuttle["passengers"]]
    for person in tmp_passengers:
        print("person[2]", person[2])
        if(person[2] == shuttle["curr_term"]):
            people_getting_onoff += 1
            shuttle["load"] -= weight[person[0]]
            shuttle["passengers"].remove(person)
            shuttle["pass_now"][person[0]] -= 1

    # adding people
    tmp_waiters = [person for person in list]
    for person in tmp_waiters:
        if(person[1] == shuttle["curr_term"]):
            if(shuttle["load"] + weight[person[0]] <= shuttle["capacity"]):
                people_getting_onoff += 1
                shuttle["load"] += weight[person[0]]
                shuttle["pass_had"][person[0]] += 1
                shuttle["pass_now"][person[0]] += 1
                shuttle["passengers"].append(person)
                list.remove(person)
    if(people_getting_onoff > 4):
        time.sleep( (people_getting_onoff-4) * 3 * tcoeff )

def moveto(term):
    shuttle["status"] = "MOVING"
    time.sleep( abs(shuttle["curr_term"]-term) * 30 * tcoeff )
    shuttle["status"] = "PARKED"
    time.sleep( 10 * tcoeff )
    shuttle["curr_term"] = term

    # set direction. shuttle moves 1-2-3-4-5-4-3-2-1...
    #                              f-f-f-f-r-r-r-r-f...
    if(shuttle["curr_term"] == 5):
        shuttle["direction"] = "reverse"
    elif(shuttle["curr_term"] == 1):
        shuttle["direction"] = "forward"

    # update dest_term accordingly
    if(shuttle["direction"] == "forward"):
        shuttle["dest_term"] = shuttle["curr_term"]+1
    elif(shuttle["direction"] == "reverse"):
        shuttle["dest_term"] = shuttle["curr_term"]-1
