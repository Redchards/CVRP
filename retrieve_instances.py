import httplib2
from bs4 import BeautifulSoup, SoupStrainer
import re
import os
import errno
import sys

# gnuplot -p <(echo -e 'set terminal jpeg; set output "plot1.jpg"; plot "-" using 1:2\n0 0 \n1 2\n2 34\n4 45\e')
# gnuplot -p <(echo -e 'set terminal jpeg; set output "plot1.jpg"; plot "-" using 1:2 with lines lc rgb "black" lw 2 notitle,"" using 1:2:(0.3) with circles fill solid lc rgb "black" notitle, "" using 1:2:3 with labels tc rgb "white" offset (0,0) notitle\n0 0 \n1 2\n2 \n0 0 \n1 2\n2 34\n4 45\ne\n0 0 \n1 2\n2 34\n4 45\ne\n0 0 1\n1 2 4\n2 34 2\n4 45 3\ne') 
# gnuplot -p <(echo -e 'set terminal jpeg; set output "plot1.jpg"; plot "-" using 1:2 with lines lc rgb "black" lw 2 notitle,"" using 1:2:(0.3) with circles fill solid lc rgb "black" notitle, "" using 1:2:3 with labels tc rgb "white" offset (0,0) notitle, "" using 1:2 with circles fill solid lc rgb "blue" ps variable notitle\n0 0 \n1 2\n2 \n0 0 \n1 2\n2 34\n4 45\ne\n0 0 \n1 2\n2 34\n4 45\ne\n0 0 1\n1 2 4\n2 34 2\n4 45 3\ne\n1 15 \n1 2\n2 23\ne')


base_url = "http://vrp.atd-lab.inf.puc-rio.br"
instances_page = base_url + "/index.php/en/"

def retrieve_instances_from(website_url):
    http = httplib2.Http()
    status, response = http.request(website_url)

    vrp_expr = r'\.vrp|\.dat'
    sol_expr = r'\.sol'
    vrp_re = re.compile(vrp_expr)
    sol_re = re.compile(sol_expr)

    vrp_list = []
    sol_list = []

    for link in BeautifulSoup(response, "html.parser", parse_only=SoupStrainer('a', {'href': re.compile('|'.join([vrp_expr, sol_expr]))})):
        hr = link['href']

        if vrp_re.search(hr):
            vrp_list.append(hr)
        elif sol_re.search(hr):
            sol_list.append(hr)

    return vrp_list, sol_list

def create_directory_if_not_exist(dirName):
    if not os.path.exists(dirName):
        try:
            os.makedirs(dirName)
        except OSError as e:
            print("Failed to create directory")
            print(e)

def download_file(file_url, file_path):
    status, response = httplib2.Http().request(file_url)

    with open(file_path, 'wb+') as f:
        f.write(response)

def download_instances(instances_links, solutions_links, instance_dir="instances", instances_repository=base_url):
    create_directory_if_not_exist(instance_dir)
    extension_expr = r'\.\w*'

    assert len(instances_links) == len(solutions_links), "The number of instances and solutions must match !"
    total_batch_size = len(instances_links)
    progressbar_length = 20
    progressbar_refresh = total_batch_size // progressbar_length

    for iter_num, (vrp_link, sol_link) in enumerate(zip(instances_links, solutions_links)):
        split_path = vrp_link.split('/')
        

        file_dir, vrp_filename = split_path[4], re.sub(extension_expr, '.vrp', split_path[5])
        sol_filename = re.sub(extension_expr, '.sol', sol_link.split('/')[5])

        file_path = os.path.join(instance_dir, file_dir)
        create_directory_if_not_exist(file_path)

        download_file(instances_repository + vrp_link, os.path.join(file_path, vrp_filename))
        download_file(instances_repository + sol_link, os.path.join(file_path, sol_filename))

        progressbar_idx = iter_num // progressbar_refresh
        sys.stdout.write("[" + "#" * progressbar_idx + ">" + " " * (progressbar_length - progressbar_idx) + "] : ")
        sys.stdout.write("{0:.2f}%\r".format(float(iter_num) / total_batch_size * 100))
        sys.stdout.flush()

    print("[" + "#" * progressbar_length + ">] : 100%   ")



if __name__ == "__main__":
    print("Parsing {} ...".format(instances_page))
    instances_links, solutions_links = retrieve_instances_from(instances_page)

    print("Found {} instances !".format(len(instances_links)))
    print("Retrieving the instances from the web ... This operation might take a while !")
    download_instances(instances_links, solutions_links)
    print("Download complete !")
