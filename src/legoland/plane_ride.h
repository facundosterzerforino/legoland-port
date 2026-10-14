#pragma once

struct ClassNode;
struct Element;

struct PlaneRideNode;
void FUN_0043d9f0(struct PlaneRideNode *node);
void FUN_0043d990(struct PlaneRideNode *node);
void PlaneRideUpdate(struct Element *elem);

struct Ride;
void PlaneRide_GetInterfaces(struct ClassNode *name, struct Ride *iface);
