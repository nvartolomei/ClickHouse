import pytest

from helpers.cluster import ClickHouseCluster

cluster = ClickHouseCluster(__file__)

instance = cluster.add_instance(
    'instance',
    user_configs=[
        "configs/users.d/assign_my_pool_1.xml",
        "configs/users.d/resource_pool.xml",
        "configs/users.d/allow_introspection_functions.xml"
    ]
)


@pytest.fixture(scope="module", autouse=True)
def started_cluster():
    try:
        cluster.start()
        yield cluster

    finally:
        cluster.shutdown()


def test_quota_from_users_xml():
    print(instance.query("SHOW POOLS"))
    print(instance.query("SHOW RESOURCE POOLS"))
    print(instance.query("SHOW CURRENT RESOURCE POOL"))
    print(instance.query("SHOW CREATE RESOURCE POOL"))
    print(instance.query("SELECT * FROM system.resource_pools"))

    print("user_1")
    print(instance.query("SHOW CURRENT RESOURCE POOL", user="user_1"))
