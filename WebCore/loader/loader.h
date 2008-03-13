/*
    Copyright (C) 1998 Lars Knoll (knoll@mpi-hd.mpg.de)
    Copyright (C) 2001 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2004, 2006, 2007, 2008 Apple Inc. All rights reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef loader_h
#define loader_h

#include "PlatformString.h"
#include "StringHash.h"
#include "SubresourceLoaderClient.h"
#include "Timer.h"
#include <wtf/Deque.h>
#include <wtf/HashMap.h>
#include <wtf/Noncopyable.h>

namespace WebCore {

    class CachedResource;
    class DocLoader;
    class Request;

    class Loader : Noncopyable {
    public:
        Loader();
        ~Loader();

        void load(DocLoader*, CachedResource*, bool incremental = true, bool skipCanLoadCheck = false, bool sendResourceLoadCallbacks = true);

        void cancelRequests(DocLoader*);
        
        enum Priority { Low, Medium, High };
        void servePendingRequests(Priority minimumPriority = Low);

    private:
        Priority determinePriority(const CachedResource*) const;
        void scheduleServePendingRequests();
        
        void requestTimerFired(Timer<Loader>*);

        class Host : private SubresourceLoaderClient {
        public:
            Host(unsigned maxRequestsInFlight);
            ~Host();
            
            void addRequest(Request*, Priority);
            void servePendingRequests(Priority minimumPriority = Low);
            void cancelRequests(DocLoader*);
            bool hasRequests() const;
        
        private:
            virtual void didReceiveResponse(SubresourceLoader*, const ResourceResponse&);
            virtual void didReceiveData(SubresourceLoader*, const char*, int);
            virtual void didFinishLoading(SubresourceLoader*);
            virtual void didFail(SubresourceLoader*, const ResourceError&);
            
            typedef Deque<Request*> RequestQueue;
            void servePendingRequests(RequestQueue& requestsPending);
            void didFail(SubresourceLoader*, bool cancelled = false);
            void cancelPendingRequests(RequestQueue& requestsPending, DocLoader*);
            
            RequestQueue m_requestsPending[High + 1];
            typedef HashMap<RefPtr<SubresourceLoader>, Request*> RequestMap;
            RequestMap m_requestsLoading;
            const int m_maxRequestsInFlight;
        };
        typedef HashMap<String, Host*> HostMap;
        HostMap m_hosts;
        Host m_nonHTTPProtocolHost;
        
        Timer<Loader> m_requestTimer;
    };

}

#endif
