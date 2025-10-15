Question 1: Time Travel Debugging
Your NTP client reports your clock is 30 seconds ahead, but you just synchronized yesterday. List three possible causes and how you'd investigate each one. Consider both technical issues (hardware, software, network) and real-world scenarios that could affect time synchronization.
- One possible cause could be due to hardware issues. One example of this would be using a battery-powered laptop, where the internal clock on the laptop is powered with the battery. This could be one leading cause time to drift and become unsynchronized. This can be a very real example, as most people own laptops, and people such as myself with laptops with dedicated GPU's can experience their laptops failing to hold battery for any extended period of time.

- Another possible cause could be due to software. For example, within a computer's settings, you have the ability to see the time server being used. On Mac, the default time server is time.apple.com, on Windows, time.windows.com. By default, this is how computers synchronize time. At the same time, within those settings, users can turn off the automatic time synchonization, which can lead to drift, as the computer won't attempt to re-synchronize.

- One additional cause for time unsynchronization can be failure to connect to a network. If a computer becomes disconnected from a network, it will no longer become able to synchronize. On the other hand, if the time server being used by the computer goes down for some reason, again, the computer won't be able to syncchronize the time and therefore become unsynchronized.

Question 2: Network Distance Detective Work
Test your NTP client with two different servers - one geographically close to you (like a national time service) and one farther away. Compare the round-trip delays you observe.

Based on your results, explain why the physical distance to an NTP server affects time synchronization quality. Why might you get a more accurate time sync from a "worse" time source that's closer to you rather than a "better" time source that's farther away? What does this tell us about distributed systems in general?

Include your actual test results and delay measurements in your answer.

- With server closer: time.nist.gov:
~~~
    Received NTP response from time.nist.gov!
    --- Response Packet ---
    Leap Indicator: 0
    Version: 3
    Mode: 4
    Stratum: 1
    Poll: 13
    Precision: -29
    Reference ID: [0x4E495354] NIST
    Root Delay: 16
    Root Dispersion: 32
    Reference Time: 2025-10-15 17:05:04.000000 (Local Time)
    Original Time (T1): 2025-10-15 17:05:59.066537 (Local Time)
    Receive Time (T2): 2025-10-15 17:05:59.090150 (Local Time)
    Transmit (T3) Time: 2025-10-15 17:05:59.090151 (Local Time)

    === NTP Time Synchronization Results ===
    Server: time.nist.gov
    Server Time: 2025-10-15 17:05:59.090151
    Local Time: 2025-10-15 17:05:59.113776
    Round Trip Delay: 0.047238

    Time offset: -0.000006
    Final Dispersion: 0.02361
~~~

- With server far away: asia.pool.ntp.org:
~~~
    Received NTP response from asia.pool.ntp.org!
    --- Response Packet ---
    Leap Indicator: 0
    Version: 4
    Mode: 4
    Stratum: 2
    Poll: 6
    Precision: -24
    Reference ID: [0x11FD747D] 17.253.116.125
    Root Delay: 9
    Root Dispersion: 38
    Reference Time: 2025-10-15 17:02:39.896584 (Local Time)
    Original Time (T1): 2025-10-15 17:04:18.351115 (Local Time)
    Receive Time (T2): 2025-10-15 17:04:18.445495 (Local Time)
    Transmit (T3) Time: 2025-10-15 17:04:18.445496 (Local Time)

    === NTP Time Synchronization Results ===
    Server: asia.pool.ntp.org
    Server Time: 2025-10-15 17:04:18.445496
    Local Time: 2025-10-15 17:04:18.535090
    Round Trip Delay: 0.183974

    Time offset: 0.002393
    Final Dispersion: 0.091987
~~~    

- From these results, we see that the delay for a "closer" server is .047238, and the further away server, in Asia, the delay is much greater at .183974. Additionally, it is good to point out the time offset's in both of these, with the US-based server coming in at +/- 23.62ms, and the Asia-based server at 99.99ms - quite a large difference! Overall, this shows that distance also plays a large factor in distributed systems. Just because a time server is considered "good" might not necessarily mean that it is the best option for the optimum synchronization. The further away a server is, the more hops the packet must jump through in order to get to it's destination, leading to larger delays in reception of a packet, and lowering the quality of the data received.


Question 3: Protocol Design Challenge
Imagine a simpler time protocol where a client just sends "What time is it?" to a server, and the server responds with "It's 2:30:15 PM".

Explain why this simple approach wouldn't work well for accurate time synchronization over a network. In your answer, discuss what problems network delay creates for time synchronization and why NTP needs to exchange multiple timestamps instead of just sending the current time. What additional information does having all four timestamps (T1, T2, T3, T4) provide that a simple request-response couldn't?

- This simple protocol would not work due to network delay. The time recieved from the server would be the time that the server sent the response, not the current time accounting for network delay/travel, so there is no way to actually know what the real time is. As we have learned this is why it is important to include additional information with 4 different time stamps. The first timestamp, T1, is the time at which the client sends the request. The second timestamp, T2, is the time the server received the timestamp. The third timestamp T3, is the time the server sends the response, and the fourth, T4, is when the client receives the response. All of these together serve an important purpose, and allow NTP to calculate the true, accurate time. For example, delay can be calculated with the formula: (T4 - T1) - (T3 - T2). This will give us the amount of time it took for the message to be sent and come back. The additional information also allows us to calculate how far off the host's clock is in comparison to the server. Overall, with the simple protocol, there is no way to get an accurate representation of the current time, whereas with NTP, it is made possible with the additional information provided and the calculations that can be performed with that information.