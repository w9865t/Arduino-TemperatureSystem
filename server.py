import socket
import schedule
import multiprocessing
import getweather as gw
import discord
from discord.ext import commands

# discord function
def discord_addon(shared_list):
    intents = discord.Intents.all()
    intents.members = True
    bot = commands.Bot(command_prefix='?', intents=intents)

    @bot.event
    async def on_ready():
        print(f'Login bot: {bot.user}')

    @bot.command()
    async def 温度(ctx):
        await ctx.send(f'現在気温は{round(shared_list[0], 2)}℃です。\n室内の温度は{round(shared_list[1], 2)}℃です。\n温度差は{round(abs(shared_list[1] - shared_list[0]), 2)}℃です。')

    bot.run('自分のDiscord APIキーを入力')

# comm function
def arduino_comm(shared_list):
    #host = "192.168.11.4"
    host = "192.168.0.4"
    port = 9999

    sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR, 1)
    sock.bind((host,port))
    sock.listen(1)

    
    room_temp = 9999.99
    online_temp = 9999.99

    # cache online temp data
    def cache_temp_online():
        nonlocal online_temp
        online_temp = gw.getWeather()
        # update shared list
        shared_list[0] = online_temp
        # for test
        print("Obtained data from online. current online temp is " + str(online_temp))

    # room temp update
    def room_temp_update(msg, room_temp):
        room_temp = msg.decode('ascii')
        # update shared list
        shared_list[1] = float(room_temp)

        print("Obtained data from Arduino. current room temp is " + str(room_temp))
    
    # schedule caching temp from online, run once
    schedule.every(5).seconds.do(cache_temp_online)
    online_temp = cache_temp_online()

    while True:
        cs,caddr = sock.accept()

        with cs:
            while True:
                # run scheduled caching
                schedule.run_pending()

                # receiving room temp
                msg = cs.recv(1024)
                if not msg:
                    break
                #print(msg.decode('ascii'))
                room_temp_update(msg, room_temp)

                # send online temp
                cs.sendall(bytes(str(online_temp), 'ascii'))

    sock.close()


if __name__ == '__main__':
    #multiprocessing.set_start_method('spawn')
    manager = multiprocessing.Manager()
    shared_list = manager.Array('d', [9999.99, 9999.99])
    # 0 is online, 1 is room



    arduino_comm_process = multiprocessing.Process(target=arduino_comm, args=[shared_list])
    discord_process = multiprocessing.Process(target=discord_addon, args=[shared_list])
    arduino_comm_process.start()
    discord_process.start()

    arduino_comm_process.join()
    discord_process.join()