/**
  \file Server.cpp
  \brief UDP server interface class for connecting to the Pidar.
  \author Jonathan Ulrich (jongulrich@gmail.com)
  \author Andrew Watson (watsontandrew@gmail.com)
  \date 2014
*/
#include "PointStructs.hpp"
#include "Point3D.hpp"
#include "Control.hpp"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/serialization/vector.hpp>
#include <vector>
#include <iostream>

using namespace Pidar;


Server::Server(boost::asio::io_service& io_service,
               const boost::asio::ip::address& multicast_address)
    : mEndpoint(multicast_address, multicast_port),
      mSocket(io_service, mEndpoint.protocol()),
      mDeadlineTimer(io_service)
{
    mPointClouds.clear();
    pcl_data w;
    pcl_point x;
    x.r = 0.0;
    x.theta = 0.0;
    x.phi = 0.0;
    x.intensity = 0.0;
    x.newScan = false;
    w.points.push_back(x);
    mPointClouds.push_back(w);
    std::cout << "Sending Data"
              << std::endl;
    std::cout << "on "
              << mEndpoint.address()
              << ":" << mEndpoint.port()
              << std::endl;
    mSocket.async_send_to(
                boost::asio::buffer(mPointClouds), mEndpoint,
                boost::bind(&Server::handle_send_to, this,
                            boost::asio::placeholders::error));
}


void Server::handle_send_to(const boost::system::error_code& error)
{
    if(!error)
    {
        mDeadlineTimer.expires_from_now
                (boost::posix_time::milliseconds(transmission_delay_ms));
        mDeadlineTimer.async_wait(
                    boost::bind(&Server::handle_timeout, this,
                                boost::asio::placeholders::error));
    }
}


void Server::handle_timeout(const boost::system::error_code& error)
{
    if(!error)
    {
        mPointClouds.clear();
        if(gSendPoints.size() > 5)
        {
            gSendPoints.clear();
        }
        if(!gSendPoints.empty())
        {
            mPointClouds.push_back(gSendPoints.front());
            gSendPoints.pop_front();
            mSocket.async_send_to(
                        boost::asio::buffer(mPointClouds[0].points),
                    mEndpoint,
                    boost::bind(&Server::handle_send_to,
                                this,
                                boost::asio::placeholders::error));
        }
        else
        {
            handle_send_to(error);
        }
    }
}
/* End of File */
