from fastapi import FastAPI
from pydantic import BaseModel

app = FastAPI()

class Message(BaseModel):
    message: str

@app.post("/message")
async def receive_message(message: Message):
    print("Received message:", message.message)
    return {"status": "success"}