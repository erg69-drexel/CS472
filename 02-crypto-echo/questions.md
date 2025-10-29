# Crypto Echo - Concept Questions

## Instructions

Answer the following questions to demonstrate your understanding of the networking concepts and design decisions in this assignment. Your answers should be thoughtful and demonstrate understanding of the underlying principles, not just surface-level descriptions.

**Submission Requirements:**
- Answer all 5 questions
- Each answer should be 1-2 paragraphs (150-300 words)
- Use specific examples from the assignment when applicable
- Explain the "why" behind design decisions, not just the "what"

---

## Question 1: TCP vs UDP - Why Stateful Communication Matters

**Question:**
This assignment requires you to use TCP instead of UDP. Explain in detail **why TCP is necessary** for this encrypted communication application. In your answer, address:
- What specific features of TCP are essential for maintaining encrypted sessions?
- What would break or become problematic if we used UDP instead?
- How does the stateful nature of TCP support the key exchange and encrypted communication?

**Hint:** Think about what happens to the encryption keys during a session and what TCP guarantees that UDP doesn't.

```
TCP is necessary for this assignment due to its reliable and ordered stream protocol. This means that the protocol ensures that packets are not only delivered, but delivered in order, making it essential for secure (encrypted) communications. If data is lost in transmission, then things like secure keys can be lost or delivered out of order, leading to errors when another party is decrypting. Additionally, because TCP is a stream protocol, this means that once a connection is established between a server and client, that connection remains open until one of them disconnects. This is crucial for secure communications, as it allows for one key to be generated and used throughout the communication until it is closed.

UDP on the other hand would not be ideal for something like a secure communication program. UDP does not guarantee delivery of all data, making it not desirable for secure communication, due to the fact that if keys are lost, the communication won’t work. Because it is not a “reliable” transmission protocol, we would also have to handle a lot of the error checking and network packet handling, which can make things much more complicated.
```

---

## Question 2: Protocol Data Unit (PDU) Structure Design

**Question:**
Our protocol uses a fixed-structure PDU with a header containing `msg_type`, `direction`, and `payload_len` fields, followed by a variable-length payload. Explain **why we designed the protocol this way** instead of simpler alternatives. Consider:
- Why not just send raw text strings like "ENCRYPT:Hello World"?
- What advantages does the binary PDU structure provide?
- How does this structure make the protocol more robust and extensible?
- What would be the challenges of parsing messages without a structured header?

**Hint:** Think about different types of data (text, binary, encrypted bytes) and how the receiver knows what it's receiving.

```
 Having a fixed PDU-structure with a header makes this program much simpler, contrary to the example provided. For example, the payload_len field helps with ensuring that all data is fully received by recv(), and allows us to loop around and continue to recv() until the bytes received is equal to the payload length. In the example provided, with the raw text strings, you would not be able to tell when all data has been received, which can lead to missing data. Additionally, because we are also using encryption for this assignment, raw text is not the only format being used. When data is encrypted, it is not guaranteed to be sent as a raw text string, which can lead to parsing errors later on. 

Having a msg_type field is also extremely useful, as it allows for easy command handling, there is no need to parse anything, you can simply look at the header of the packet and see if it is message data, requesting an encryption key, or sending encrypted information. From there it is as simple as an if/else or switch case on the message type.
```

---

## Question 3: The Payload Length Field

**Question:**
TCP is a **stream-oriented protocol** (not message-oriented), yet our PDU includes a `payload_len` field. Explain **why this field is critical** even though TCP delivers all data reliably. In your answer, address:
- How does TCP's stream nature differ from message boundaries?
- What problem does the `payload_len` field solve?
- What would happen if we removed this field and just relied on TCP?
- How does `recv()` work with respect to message boundaries?

**Hint:** Consider what happens when multiple PDUs are sent in rapid succession, or when a large PDU arrives in multiple `recv()` calls.

```
Because TCP is stream-oriented, the receiver of a message gets a continuous flow of bytes. As a result of this, the receiver has no way of knowing when one message ends and another begins. The payload_len field is extremely important as it allows the receiver to first look at the header of the packet, and determine how many bytes it needs to receive. From there, the receiver can call recv() again, and loop around, counting the number of bytes received each time, until the number of bytes received is equal to the payload length, delivered in the header.

In the case of multiple PDUs sent in rapid succession, without knowing the length of each PDU payload, multiple PDUs can be merged together, since you cannot ensure each payload is received fully, as you don’t know the payload length, and therefore cannot loop around and call recv() multiple times on one payload until it is fully received.

```

---

## Question 4: Key Exchange Protocol and Session State

**Question:**
The key exchange must happen **before** any encrypted messages can be sent, and keys are **session-specific** (new keys for each connection). Explain **why this design is important** and what problems it solves. Address:
- Why can't we just use pre-shared keys (hardcoded in both client and server)?
- What security or practical benefits come from generating new keys per session?
- What happens if the connection drops after key exchange? Why is this significant?
- How does this relate to the choice of TCP over UDP?

**Hint:** Think about what "session state" means and how it relates to the TCP connection lifecycle.

```
Key exchange taking place before any encrypted messages are sent and keys being session specific are extremely important when it comes to secure messaging. 

Hardcoding client and server keys with pre-shared keys essentially negates any security that would be taking place if they were generated keys per session. While it may protect against message interception, all other protections are theoretically thrown out the window. For example, if one client was attacked and compromised, then essentially all other clients would also be compromised, as they all share the same key. 

Generating new keys per session allows for single use keys per connection, which thoroughly enhances the security of the program. Compromise clients are isolated and additional clients would have to be compromised one-by-one, rather than them all behind compromised together. Once a client disconnects, the key is no longer valid, and there is no risk for clients to be compromised with that key. Additionally, when a connection is dropped after key exchange, if there is one shared key, then anyone could hijack the session and perform malicious actions. With single-session keys, if a session is hijacked, then the malicious attacker would still not be able to do anything, as the server would no longer be using that session key.

As for the choice of TCP vs UDP, a server with UDP does not know which client it is communicating with, and can not have a session key assigned to each client. On the other hand, with TCP, the server keeps track of which session key belongs to which client. Additionally, as mentioned previously, with UDP, keys could get lost in transmission as it does not guarantee delivery, which would lead to encryption/decryption errors between the client and server.

```

---

## Question 5: The Direction Field in the PDU Header

**Question:**
Every PDU includes a `direction` field (DIR_REQUEST or DIR_RESPONSE), even though the client and server already know their roles. Some might argue this field is redundant. Explain **why we include it anyway** and what value it provides. Consider:
- How does this field aid in debugging and protocol validation?
- What would happen if you accidentally swapped request/response handling code?
- How does it make the protocol more self-documenting?
- Could this protocol be extended to peer-to-peer communication? How does the direction field help?

**Hint:** Think about protocol clarity, error detection, and future extensibility beyond simple client-server.

```
Firstly, having the direction field for request or response in the PDU header is extremely useful for debugging. For example, in this assignment, we were provided the function “print_msg_info()”, which displays the command type, payload length, as well as the direction. For debugging, you can easily see what is being sent from where, which is especially useful when having to look at how many bytes are sent to the server, and how many are received from the server.

If you were to swap request and response handling code, error handling could be implemented on the server to check if the incoming message is a request or response, and you could do the same for a client. This would ensure that both are always receiving the correct type of packet, and won’t try to process incorrect packets.

While this might seem redundant, if we were to implement peer-to-peer, it may not be as cut and dry. While in this program, the client will be sending requests, and the server will be sending responses, that same logic might not apply in peer-to-peer. They could be sending commands back and forth, which could lead to confusion if the direction is unknown. 

```

---

## Evaluation Criteria

Your answers will be evaluated on:

1. **Technical Accuracy** (40%)
   - Correct understanding of networking concepts
   - Accurate description of TCP/UDP differences
   - Proper explanation of protocol design principles

2. **Depth of Understanding** (30%)
   - Going beyond surface-level descriptions
   - Connecting concepts to specific implementation details
   - Demonstrating "why" not just "what"

3. **Completeness** (20%)
   - Addressing all parts of each question
   - Providing specific examples
   - Covering edge cases or potential issues

4. **Clarity** (10%)
   - Well-organized answers
   - Clear technical writing
   - Proper use of terminology

---

## Sample Strong Answer Structure

Here's an example of how to structure a strong answer for Question 1:

> **Opening Statement:** TCP is essential for this application because it provides a stateful, 
> reliable connection that maintains the encryption session state throughout the communication.
>
> **Key Points with Explanation:**
> - TCP's connection-oriented nature means the encryption keys, once exchanged, remain valid 
>   for the entire session. With UDP, each datagram is independent, so we'd need to either 
>   re-exchange keys with every message (inefficient and insecure) or somehow track session 
>   state externally (complex).
>
> - TCP guarantees reliable, ordered delivery. If a key exchange message was lost (as could 
>   happen with UDP), the client and server would be out of sync - the client might try to 
>   encrypt with a key it thinks it has, but the server never received the exchange request.
>
> - The stateful connection allows us to assume that once keys are exchanged, they persist 
>   until the connection closes. This is fundamental to the protocol design.
>
> **Conclusion/Summary:** Without TCP's stateful guarantees, we'd need to completely redesign 
> our protocol to handle key management, message reliability, and session tracking - essentially 
> reimplementing TCP's features at the application layer.

---

## Submission

Submit your answers as:
- A text file: `lastname_firstname_answers.txt`
- Or a PDF: `lastname_firstname_answers.pdf`
- Or a Markdown file: `lastname_firstname_answers.md`

Include your name and student ID at the top of your submission.

**Due Date:** [To be announced by instructor]

---

## Additional Hints

If you're stuck on a question, try these approaches:

1. **Review the code:** Look at how the protocol is actually implemented
2. **Think about failure cases:** What would break if we did it differently?
3. **Consider alternatives:** Why didn't we use simpler approaches?
4. **Trace the flow:** Follow a message from client through network to server
5. **Read the documentation:** Check `crypto.md` and `README.md` for context

Remember: These questions are about understanding **why** we made certain design decisions, not just describing **what** the code does.
