#!/usr/bin/env python3
"""Static integrity checks for provisional Mission 1 spatial contracts.

Run: python tools/validate_mission_01_layout.py
This validates authored data, not collision, navmesh, sight or gameplay parity.
"""
import json
from collections import deque
from pathlib import Path

PATH = Path(__file__).resolve().parents[1] / "data" / "mission_01_spatial_blueprint.json"


def validate(document):
    errors = []
    rooms = {r["id"]: r for r in document["rooms"]}
    if len(rooms) != len(document["rooms"]):
        errors.append("Duplicate room ID")
    for room in rooms.values():
        x, y, w, h = room["bounds_xywh_m"]
        if w <= 0 or h <= 0:
            errors.append(f"Invalid room bounds: {room['id']}")
        for n in room["neighbors"]:
            if n not in rooms:
                errors.append(f"Missing room neighbor {room['id']} -> {n}")
    graph = {rid: set() for rid in rooms}
    portal_pairs = set()
    for portal in document["portals"]:
        a, b = portal["from"], portal["to"]
        if a not in rooms or b not in rooms:
            errors.append(f"Portal endpoint missing: {a} -> {b}")
            continue
        if a == b:
            errors.append(f"Self portal: {a}")
        pair = tuple(sorted((a, b)))
        if pair in portal_pairs:
            errors.append(f"Duplicate portal: {a} - {b}")
        portal_pairs.add(pair)
        graph[a].add(b)
        if portal.get("bidirectional"):
            graph[b].add(a)
        if b not in rooms[a]["neighbors"] or a not in rooms[b]["neighbors"]:
            errors.append(f"Nonreciprocal adjacency: {a} - {b}")
    for room in rooms.values():
        for n in room["neighbors"]:
            if n in rooms and tuple(sorted((room["id"], n))) not in portal_pairs:
                errors.append(f"Neighbor lacks portal: {room['id']} - {n}")
    entry, end = document["entry_area"], document["exit_area"]
    if entry not in rooms or end not in rooms:
        errors.append("Missing entry or exit room")
    else:
        visited = {entry}
        queue = deque([entry])
        while queue:
            for nxt in graph[queue.popleft()]:
                if nxt not in visited:
                    visited.add(nxt)
                    queue.append(nxt)
        if visited != set(rooms):
            errors.append(f"Disconnected rooms from entry: {sorted(set(rooms)-visited)}")
        if end not in visited:
            errors.append("Exit not reachable ignoring conditional gates")
    def inside(room_id, pt):
        x, y, w, h = rooms[room_id]["bounds_xywh_m"]
        return x <= pt[0] <= x+w and y <= pt[1] <= y+h
    for kind, entries in [("guard", document["guards"]), ("pickup", document["pickups"])]:
        ids = [e["id"] for e in entries]
        if len(ids) != len(set(ids)):
            errors.append(f"Duplicate {kind} ID")
        for item in entries:
            room = item["room"]
            if room not in rooms:
                errors.append(f"{kind} in missing room: {item['id']}")
                continue
            positions = [item["xy_m"]] if kind == "pickup" else [item["spawn_xy_m"], *item["patrol_xy_m"]]
            for pt in positions:
                if not inside(room, pt):
                    errors.append(f"{kind} position outside room: {item['id']} @ {pt}")
    for beat in document["beats"]:
        if beat["room"] not in rooms:
            errors.append(f"Beat in missing room: {beat['id']}")
    return errors


def main():
    data = json.loads(PATH.read_text(encoding="utf-8"))
    errors = validate(data)
    if errors:
        for error in errors:
            print("FAIL:", error)
        raise SystemExit(1)
    print(f"PASS Mission 1: {len(data['rooms'])} rooms, {len(data['portals'])} portals, "
          f"{len(data['guards'])} guards, {len(data['pickups'])} pickups.")


if __name__ == "__main__":
    main()
