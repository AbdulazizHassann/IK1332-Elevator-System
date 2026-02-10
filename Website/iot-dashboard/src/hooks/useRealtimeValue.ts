import { useEffect, useState } from "react";
import { onValue, ref } from "firebase/database";
import type { DatabaseReference } from "firebase/database";
import { db } from "../firebase";

type Result<T> = {
  data: T | null;
  loading: boolean;
  error: string | null;
};

export function useRealtimeValue<T>(path: string): Result<T> {
  const [data, setData] = useState<T | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    setLoading(true);
    setError(null);

    const r: DatabaseReference = ref(db, path);

    const unsubscribe = onValue(
      r,
      (snapshot) => {
        setData(snapshot.val() as T | null);
        setLoading(false);
      },
      (err) => {
        setError(err.message);
        setLoading(false);
      },
    );

    return () => unsubscribe();
  }, [path]);

  return { data, loading, error };
}
